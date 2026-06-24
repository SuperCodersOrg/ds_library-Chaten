# Redis Lite — Design Document

---

## Table of Contents

1. [Introduction](#1-introduction)
   - 1.1 [What is Redis Lite](#11-what-is-redis-lite)
   - 1.2 [Why Build It](#12-why-build-it)
   - 1.3 [What Redis Lite Is Not](#13-what-redis-lite-is-not)
2. [Architecture](#2-architecture)
   - 2.1 [The Layered Design](#21-the-layered-design)
   - 2.2 [Member Variables](#22-member-variables)
   - 2.3 [Class Invariants](#23-class-invariants)
3. [The run() Loop — Internal Flow](#3-the-run-loop--internal-flow)
   - 3.1 [Input Parsing](#31-input-parsing)
   - 3.2 [Token Dispatch](#32-token-dispatch)
   - 3.3 [Error Handling](#33-error-handling)
4. [Public API](#4-public-api)
5. [Command Deep Dive](#5-command-deep-dive)
   - 5.1 [SET](#51-set)
   - 5.2 [GET](#52-get)
   - 5.3 [DEL](#53-del)
   - 5.4 [EXISTS](#54-exists)
   - 5.5 [CLEAR](#55-clear)
   - 5.6 [EXIT](#56-exit)
6. [Time Complexity Analysis](#6-time-complexity-analysis)
7. [Complexity Proofs](#7-complexity-proofs)

---

## 1. Introduction

### 1.1 What is Redis Lite

Redis Lite is a command-line key-value store — a minimal reimplementation of the core interface of Redis (Remote Dictionary Server). It provides an interactive REPL (Read-Eval-Print Loop) that accepts string commands (`SET`, `GET`, `DEL`, `EXISTS`, `CLEAR`, `EXIT`), parses them into tokens, and dispatches each to the corresponding method on a custom `HashMap<std::string, std::string>` that acts as the sole storage engine.

At runtime, Redis Lite occupies a single `while(true)` loop. Each iteration reads one line of user input from `stdin`, tokenizes it, validates the command and its argument count, calls the appropriate `HashMap` method, and prints the result to `stdout`. The loop exits cleanly when the user sends `EXIT`.

### 1.2 Why Build It

Writing a data structure in isolation verifies that its API compiles and that its basic operations return correct values. It does not verify that the structure holds up under a real workload. Redis Lite creates that workload:

- **Missing-key handling**: `GET` and `DEL` on nonexistent keys must not crash. The application layer must catch the `std::out_of_range` exceptions that `HashMap::get()` and `HashMap::remove()` throw and print a user-readable error instead.
- **Rapid mutation**: Alternating `SET` and `DEL` on the same key stresses the insert-then-remove path, ensuring `elementCount` stays accurate and load-factor calculations remain correct after deletions.
- **Type coercion**: Redis stores everything as strings. Redis Lite enforces `HashMap<std::string, std::string>`, which means every value — numbers, booleans, serialized structures — must be stringified by the caller. This validates that the template instantiation compiles correctly for `std::string`.
- **Application interaction loop**: The `run()` loop exercises the map under an indefinite number of operations without restart, catching any state-corruption bugs that only appear after many sequential mutations.

### 1.3 What Redis Lite Is Not

Redis Lite is a single-threaded, in-memory, non-persistent CLI tool. It makes no claims about:

- **Persistence**: Data lives only for the duration of the process. There is no `SAVE`, no AOF (Append-Only File), no RDB snapshot.
- **Networking**: Real Redis is a TCP server. Redis Lite reads from `stdin` and writes to `stdout` — it is a local CLI, not a server.
- **Concurrency**: No mutexes, no thread safety. A single thread owns the `HashMap` for the entire session.
- **Data types**: Real Redis supports lists, sets, sorted sets, bitmaps, streams. Redis Lite supports only string values.

---

## 2. Architecture

### 2.1 The Layered Design

Redis Lite is intentionally a **thin wrapper** — it adds zero storage logic of its own. Every command is a single forwarding call:

```
User Input (stdin)
       │
       ▼
  RedisLite::run()          ← Parsing, dispatch, error printing
       │
       ▼
  HashMap<string, string>   ← All storage, hashing, chaining, rehashing
       │
       ▼
  Heap (Nodes + buckets)    ← Physical memory
```

This layering is the architectural proof-of-concept: `HashMap` does not depend on `RedisLite`, and `RedisLite` does not reimplement any storage logic. The boundary is clean and testable in isolation on both sides.

### 2.2 Member Variables

```cpp
class RedisLite {
private:
    HashMap<std::string, std::string> dataStore;
};
```

`RedisLite` has exactly **one data member**: the `HashMap`. It is stack-allocated inside the `RedisLite` object (the `HashMap` object itself is 24 bytes: one `Node**` pointer + two `size_t` fields). The heap allocations — the bucket array and all nodes — are managed entirely by `HashMap`'s constructor, destructor, and `rehash()`. `RedisLite` never calls `new` or `delete` directly.

**Lifetime**: `dataStore` is constructed when the `RedisLite` object is constructed (default `HashMap()` constructor: allocates 16-bucket array, all `nullptr`) and destroyed when `RedisLite` goes out of scope (the `HashMap` destructor walks all chains, deletes all nodes, then `delete[]`s the bucket array). The RAII model means Redis Lite cannot leak the hash map's memory regardless of how `run()` exits.

### 2.3 Class Invariants

```
Invariant 1:  dataStore is always in a valid state between command dispatches
Invariant 2:  Every key stored is a non-empty std::string
Invariant 3:  run() never exits with an uncaught exception visible to the OS
Invariant 4:  After EXIT, dataStore is destroyed by the destructor — no leak
```

Invariant 3 is enforced by catching `std::out_of_range` inside the dispatch block for `GET`, `DEL`, and `EXISTS`, converting exceptions into printed error messages rather than process termination.

---

## 3. The run() Loop — Internal Flow

### 3.1 Input Parsing

Each iteration of the loop reads one complete line from `stdin`:

```cpp
std::string line;
std::getline(std::cin, line);
```

The line is tokenized by splitting on whitespace into a `std::vector<std::string> tokens`. For example:

```
Input:   "SET username alice"
Tokens:  ["SET", "username", "alice"]
```

```
Input:   "GET username"
Tokens:  ["GET", "username"]
```

Tokenization is O(L) where L is the length of the input line — one pass through each character. This is dominated by the O(L) cost of the hash function itself, so it does not change the asymptotic complexity of any command.

### 3.2 Token Dispatch

After tokenization, `tokens[0]` (converted to uppercase for case-insensitivity) is compared against the known command strings in a sequence of `if / else if` branches:

```cpp
if      (cmd == "SET")    { /* validate 3 tokens, call dataStore.set()   */ }
else if (cmd == "GET")    { /* validate 2 tokens, call dataStore.get()   */ }
else if (cmd == "DEL")    { /* validate 2 tokens, call dataStore.remove() */ }
else if (cmd == "EXISTS") { /* validate 2 tokens, call dataStore.contains() */ }
else if (cmd == "CLEAR")  { /* validate 1 token,  call dataStore.clear()  */ }
else if (cmd == "EXIT")   { /* break the loop                             */ }
else                      { /* print "Unknown command" error              */ }
```

**Argument validation** happens before the `HashMap` call. If `SET` receives fewer than 3 tokens, the error is caught at the parse layer — `HashMap::set()` is never called with an invalid argument count. This separation keeps the `HashMap` free of CLI-specific validation logic.

### 3.3 Error Handling

`HashMap::get()` and `HashMap::remove()` throw `std::out_of_range` when a key is not found. `run()` wraps each dispatch in a `try / catch`:

```cpp
try {
    std::cout << dataStore.get(tokens[1]) << "\n";
} catch (const std::out_of_range&) {
    std::cout << "(nil)\n";    // Redis convention for missing keys
}
```

This mirrors real Redis behavior: `GET nonexistent` returns `(nil)`, not a crash.

---

## 4. Public API

```cpp
class RedisLite {
private:
    HashMap<std::string, std::string> dataStore;

public:
    void run();    // Starts the interactive CLI loop; blocks until EXIT
};
```

`run()` is the only public method. The entire interface of Redis Lite is a single blocking call. The caller constructs a `RedisLite` object, calls `run()`, and when `run()` returns, the object is destroyed — cleaning up all `HashMap` memory automatically.

---

## 5. Command Deep Dive

### 5.1 SET

**Syntax**: `SET <key> <value>`

**Internal call**: `dataStore.set(tokens[1], tokens[2])`

**Behaviour**: If `key` does not exist, a new node is inserted into the hash map. If `key` already exists, its value is overwritten in-place (the existing node's `value` field is updated; no new allocation occurs). Prints `OK` on success.

**Edge cases handled**:
- Fewer than 3 tokens: print `"Usage: SET <key> <value>"` — `HashMap` not called.
- Value contains spaces: only `tokens[2]` is used; any further tokens are silently ignored. A full implementation would rejoin `tokens[2..N]` into a single value string.

**Complexity**: O(1) amortized — delegates entirely to `HashMap::set()`.

### 5.2 GET

**Syntax**: `GET <key>`

**Internal call**: `dataStore.get(tokens[1])`

**Behaviour**: Returns the value associated with `key`. If the key does not exist, `HashMap::get()` throws `std::out_of_range`; the catch block prints `(nil)`.

**Complexity**: O(1) average — delegates to `HashMap::get()`.

### 5.3 DEL

**Syntax**: `DEL <key>`

**Internal call**: `dataStore.remove(tokens[1])`

**Behaviour**: Removes the key-value pair. Prints `(integer) 1` on success (matching Redis convention). If the key does not exist, prints `(integer) 0` (caught from `std::out_of_range`).

**Complexity**: O(1) average — delegates to `HashMap::remove()`.

### 5.4 EXISTS

**Syntax**: `EXISTS <key>`

**Internal call**: `dataStore.contains(tokens[1])`

**Behaviour**: Prints `(integer) 1` if the key exists, `(integer) 0` otherwise. Unlike `GET` and `DEL`, `contains()` does not throw — it returns a `bool`. No try/catch required.

**Complexity**: O(1) average — delegates to `HashMap::contains()`.

### 5.5 CLEAR

**Syntax**: `CLEAR`

**Internal call**: `dataStore.clear()`

**Behaviour**: Deletes all key-value pairs. The bucket array is retained at its current capacity (no shrink). Prints `OK`. After `CLEAR`, the map is in the same state as a freshly constructed `HashMap` with the same `bucketCount` — all bucket slots are `nullptr`, `elementCount = 0`.

**Why O(N)**: `clear()` must call `delete` on every node. There is no shortcut — skipping the node deletions would leak every stored string's heap memory. The bucket array itself is zeroed in a second O(bucketCount) pass.

**Complexity**: O(bucketCount + N) — see proof in Section 7.

### 5.6 EXIT

**Syntax**: `EXIT`

**Internal call**: `break` (exits the `while` loop)

**Behaviour**: Terminates `run()`. Control returns to the caller. The `RedisLite` destructor runs, which calls `~HashMap()`, which calls `clear()` and `delete[] buckets`. All memory is freed. The process exits cleanly.

**Complexity**: O(N) total — not for the `break` itself (O(1)), but because the subsequent destructor call is O(N). From the user's perspective, `EXIT` feels instant; the cleanup happens after `run()` returns.

---

## 6. Time Complexity Analysis

| Command | Underlying Call | Average | Worst | Note |
|:---|:---|:---|:---|:---|
| `SET key val` | `HashMap::set()` | **O(1) amortized** | O(N) | Rehash amortized away |
| `GET key` | `HashMap::get()` | **O(1)** | O(N) | Chain length ≤ α ≤ 0.75 |
| `DEL key` | `HashMap::remove()` | **O(1)** | O(N) | Chain walk + O(1) unlink |
| `EXISTS key` | `HashMap::contains()` | **O(1)** | O(N) | Identical path to `GET` |
| `CLEAR` | `HashMap::clear()` | **O(N)** | O(N) | Must delete every node |
| `EXIT` | `break` + destructor | O(1) command | O(N) cleanup | Cleanup is post-`run()` |
| Parse + tokenize | Internal | O(L) | O(L) | L = input line length |

**Overall session complexity**: For a session of C commands, each operating on keys of average length L and a map never exceeding N elements, the total time is O(C × (L + 1)) = O(C × L). The O(L) hash function dominates the O(1) bucket dispatch for each command.

---

## 7. Complexity Proofs

### SET — Amortized O(1)

`RedisLite::SET` is a direct forward to `HashMap::set()`. The full amortized proof appears in HashMap.md Section 6.4. Summary: total cost over N insertions is bounded by 2.5N (N allocation + <1.5N rehash cost). Amortized per call: 2.5N / N = O(1). The `run()` loop adds only O(L) tokenization overhead per call — a constant for bounded key lengths. ∎

### GET / DEL / EXISTS — O(1) Average

All three forward to `HashMap` methods that traverse a bucket's collision chain. Under Simple Uniform Hashing with load factor α ≤ 0.75, the expected chain length at any bucket is α. Expected comparisons per lookup: α + 1 ≤ 1.75 = O(1). The tokenization overhead is O(L). Total average: O(L + 1) = O(L), treated as O(1) for fixed-bounded keys. ∎

### CLEAR — O(bucketCount + N)

`HashMap::clear()` iterates all `bucketCount` slots (outer loop: O(bucketCount)) and deletes all N nodes (inner loops, summed: O(N)). Since α ≤ 0.75 implies N ≤ 0.75 × bucketCount = O(bucketCount), the total simplifies to O(bucketCount). For a map that has grown to hold N elements, bucketCount ≥ N (and at most 4N/3 due to the 0.75 threshold), so O(bucketCount) = O(N). ∎

### EXIT — O(1) command, O(N) destructor

The `break` statement is a single jump instruction: O(1). The subsequent destructor (`~HashMap()`) calls `clear()` — O(N) — then `delete[] buckets` — O(1). Total post-`run()` cleanup: O(N). This cost is paid once at process termination and is unavoidable for any structure that owns heap memory. ∎

### Parse overhead does not change command complexity

Tokenizing a line of length L is O(L): one pass through each character. The hash function for a key of length K is also O(K). Since K ≤ L always, the tokenization cost is absorbed into the hash computation cost. Every command's complexity is therefore O(L + HashMap_operation) = O(L) for O(1) HashMap operations, which is O(1) for fixed-bounded key lengths. ∎