# Doubly Linked List — Design Document

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Design Evolution](#2-design-evolution)
3. [Internal Architecture](#3-internal-architecture)
4. [Memory Layout](#4-memory-layout)
5. [Pointer Mechanics](#5-pointer-mechanics)
6. [Dynamic Allocation Per Node](#6-dynamic-allocation-per-node)
7. [Constructors and Object Lifetime](#7-constructors-and-object-lifetime)
8. [Rule of Three](#8-rule-of-three)
9. [Public API](#9-public-api)
10. [Traversal Optimization](#10-traversal-optimization)
11. [Pointer Relinking — The Core Operation](#11-pointer-relinking--the-core-operation)
12. [Function-by-Function Reference](#12-function-by-function-reference)
13. [Time Complexity Analysis](#13-time-complexity-analysis)
14. [Complexity Proofs](#14-complexity-proofs)
15. [Linked List vs Dynamic Array — When to Choose Which](#15-linked-list-vs-dynamic-array--when-to-choose-which)

---

## 1. Introduction

### 1.1 What is a Doubly Linked List

A doubly linked list stores its elements in separate **nodes**, each living on its own piece of heap memory. Every node holds three things: the data itself, a `next` pointer to the node that comes after it, and a `prev` pointer to the node that comes before it. The list keeps two sentinel pointers — `head` (the first node) and `tail` (the last node) — plus a `size` counter.

At the hardware level this is a **non-contiguous** structure. Nodes can sit anywhere in memory — a node at address `0x1000` might point to one at `0x8FA0`. What makes them a sequence is purely the chain of pointers, not where they physically live.

This document covers a custom `List<T>` built with `new`/`delete` for each node. The goal is to understand *why* it works: how two pointers per node guarantee O(1) at both ends, why lookups are stuck at O(N), and the math behind every complexity claim.

### 1.2 Why Linked Lists Exist

Arrays put everything in one contiguous block. That's great for random access (O(1) by index) but bad for inserting or deleting in the middle — everything after the target has to shift over in memory.

A linked list trades that random access away in exchange for O(1) pointer relinking: inserting a node between two existing nodes only ever touches four pointers, no matter how big the list is. This makes linked lists the right choice when:

- Insertion/deletion at the ends dominates (queues, stacks, deques)
- The element count is unpredictable and changes fast
- You need O(1) splitting or merging of sequences
- Node addresses must stay fixed (no reallocation ever moves them)

### 1.3 Arrays vs Linked Lists

| Property | Dynamic Array | Doubly Linked List |
|:---|:---|:---|
| Memory layout | Contiguous | Scattered (per-node heap allocation) |
| Random access `at(i)` | **O(1)** (pointer + offset) | **O(N)** (must traverse) |
| Insert/remove at ends | O(1) amortized (array), O(1) strict (list) | **O(1) strict** |
| Insert/remove in middle | **O(N)** (shift elements) | O(N) to find + **O(1) to relink** |
| Cache performance | **Excellent** (contiguous, prefetchable) | **Poor** (pointer chasing = cache misses) |
| Memory per element | `sizeof(T)` | `sizeof(T) + 2 × sizeof(pointer)` (16 extra bytes on 64-bit) |
| Iterator stability | Invalidated on reallocation | **Stable** (node addresses never change) |
| No reallocation | No (periodic doubling) | **Yes** (each node allocated independently) |

The key asymmetry: a dynamic array has to shift N elements to insert at index 0 — that's O(N) work. A doubly linked list just adjusts 4 pointers — O(1) work. But finding position `k` in the list means walking `k` nodes one at a time — O(N) work. Finding position `k` in an array is one multiply-and-add — O(1) work.

### 1.4 Real-World Use Cases

Doubly linked lists show up anywhere O(1) end-operations or O(1) splicing matters:

- **OS process schedulers**: Linux's `task_struct` list manages runnable processes with O(1) add/remove from scheduling queues.
- **LRU caches**: combined with a hash map, moving a node to the front on access is O(1) thanks to stable pointers.
- **Text editors**: each line is a node; inserting a line between two others is O(1) relinking, not O(N) shifting.
- **Browser history**: forward/back navigation maps directly to `next`/`prev`.
- **Playlists**: next/prev track buttons walk a doubly linked playlist.
- **Undo/redo**: each state is a node; undo follows `prev`, redo follows `next`.
- **Memory allocators**: the free-list in many `malloc` implementations is a doubly linked list of free blocks.

---

## 2. Design Evolution

### 2.1 Attempt 1 — Singly Linked List

**Design**: Each node has only a `next` pointer. The list keeps a `head` pointer (and optionally `tail`).

```cpp
struct Node {
    T data;
    Node* next;    // Only forward link
};

Node* head;
Node* tail;        // optional
size_t listSize;
```

**What works**: `push_front` is O(1) — allocate, point the new node's `next` at the old head, update head. `push_back` (with a `tail` pointer) is O(1) — allocate, link `tail->next`, update tail. `pop_front` is O(1) — save `head->next`, delete head, update head.

**The fatal flaw — `pop_back` is O(N)**: to delete the last node, `tail` must move to the second-to-last node. But with only `next` pointers, there's no direct way to reach that node from `tail`. The only option is to walk the entire list from `head`:

```cpp
void pop_back() {
    Node* current = head;
    while (current->next != tail) {    // Walk O(N) steps
        current = current->next;
    }
    delete tail;
    tail = current;
    tail->next = nullptr;
    --listSize;
}
```

**Proof that `pop_back` is O(N)**: with N nodes `n₀ → n₁ → ... → n_{N-1}`, `tail` points at `n_{N-1}`. To delete it, we need a pointer to `n_{N-2}` so we can set `n_{N-2}->next = nullptr`. Starting from `head = n₀`, reaching `n_{N-2}` takes exactly N−2 steps of `current = current->next`. So `pop_back` does Θ(N) work — every single time, not just in the worst case.

| Operation | Singly Linked (with tail) | Problem |
|:---|:---|:---|
| `push_front` | O(1) | — |
| `push_back` | O(1) | — |
| `pop_front` | O(1) | — |
| `pop_back` | **O(N)** | Must traverse to find predecessor of tail |

**Verdict**: a singly linked list can't act as a deque or double-ended queue with O(1) at both ends. Missing the backward link is a fundamental architectural problem.

### 2.2 Attempt 2 — Doubly Linked List

**Design**: give each node a second pointer — `prev` — pointing backward to its predecessor.

```cpp
struct Node {
    T data;
    Node* next;
    Node* prev;    // ← The fix
};
```

Now `pop_back` is O(1):

```cpp
void pop_back() {
    Node* toDelete = tail;
    tail = tail->prev;          // Walk backward ONE step — O(1)
    if (tail) tail->next = nullptr;
    else head = nullptr;
    delete toDelete;
    --listSize;
}
```

`tail->prev` directly gives the second-to-last node. No traversal needed.

**Why this fixes everything**: every node now knows both its neighbors in O(1). Given any node `p`, you reach `p->next` and `p->prev` with a single dereference each. The list becomes **symmetrically navigable** — anything that worked efficiently from `head` now works just as well starting from `tail`.

**Cost of the fix**: each node needs 16 extra bytes (two 8-byte pointers on 64-bit). For a large `T` (say a 1 KB struct), that's nothing. For a tiny `T` like `char`, it's a 1600% memory overhead. This is the fundamental space tradeoff of doubly linked lists.

---

## 3. Internal Architecture

### 3.1 The Node Structure

```cpp
template<typename T>
struct Node {
    T data;          // The stored element
    Node* next;      // Pointer to the successor (nullptr if this is tail)
    Node* prev;      // Pointer to the predecessor (nullptr if this is head)

    Node(const T& value) : data(value), next(nullptr), prev(nullptr) {}
};
```

**`T data`**: the user's element, stored by value inside the node. The node **owns** it — deleting the node calls its destructor automatically (`delete node` calls `~Node()`, which calls `~T()` on `data`).

**`Node* next`**: an 8-byte address (on 64-bit systems) of the next node, or `nullptr` at the end.

**`Node* prev`**: an 8-byte address of the previous node, or `nullptr` at the start. This is the one thing a singly linked list doesn't have.

**Constructor**: sets `next` and `prev` to `nullptr`. A freshly built node is always isolated — the insertion function must explicitly link it in.

### 3.2 Member Variables

```cpp
template<typename T>
class List {
private:
    Node* head;         // first node, nullptr if empty
    Node* tail;         // last node, nullptr if empty
    size_t listSize;    // current node count
};
```

**`head`**: where all forward traversal starts. Updated whenever the front changes.

**`tail`**: where backward traversal, `push_back`, and `pop_back` start. Without it, every append would need to traverse to the end first — that's exactly why this pointer exists.

**`listSize`**: an exact running count, kept up to date on every insert/remove so `size()` never has to count nodes itself.

**Total size of the control structure**: on a 64-bit system, 8 + 8 + 8 = **24 bytes** — tiny, no matter how many nodes the list actually holds.

### 3.3 Ownership Semantics

`List<T>` follows a **chain ownership model**:

- `List` owns `head`.
- Each `Node` owns `node->next`.
- Each `Node` owns its own `data`.

Ownership flows down the `next` chain. To destroy the list you must traverse it and delete every node one by one — C++ doesn't cascade-delete through `next` for you.

The `prev` pointers are **non-owning back-references** — they point at nodes owned by their predecessor. Never `delete` through `prev`.

### 3.4 Class Invariants

These must hold true at all times, between any two public method calls:

```
Invariant 1:  listSize == 0 ⟺ head == nullptr ⟺ tail == nullptr
Invariant 2:  listSize == 1 ⟺ head == tail ⟺ head->next == nullptr ⟺ head->prev == nullptr
Invariant 3:  listSize ≥ 2 ⟹ head != tail
Invariant 4:  head->prev == nullptr (head has no predecessor)
Invariant 5:  tail->next == nullptr (tail has no successor)
Invariant 6:  For every interior node p: p->prev->next == p and p->next->prev == p
Invariant 7:  listSize equals the number of nodes reachable by following next from head
```

Invariant 6 is the one buggy code breaks most often. Every relinking operation must keep both directions consistent: if A's `next` is set to point at B, B's `prev` **must** be set to point back at A.

---

## 4. Memory Layout

<p align="center">
    <img src="../Memory-Diagrams/LL-Memory Design.jpeg" alt="Memory Layout of a Doubly Linked List" width="600">
</p>

---

## 5. Pointer Mechanics

### 5.1 What a Pointer Is

A pointer is a variable that holds a **memory address**. On a 64-bit system, every pointer is exactly 8 bytes — a number that names a location in memory.

```
Node* p = new Node(42);

p:    [0x0000'7FFF'0000'1A20]    ← 8 bytes holding an address
                    ↓
            Memory at 0x1A20:
            [42][0x0000...0000][0x0000...0000]
             ↑data              ↑next=null      ↑prev=null
```

When you write `p->data`, the CPU: (1) reads the 8-byte address inside `p`, (2) goes to that address, (3) reads `sizeof(T)` bytes from there.

When you write `p->next`, the CPU: (1) reads `p` to get the address, (2) jumps to `address + sizeof(T)` (the offset of `next` inside the struct), (3) reads 8 bytes — which is itself another address.

This chasing of address → address → address is called **pointer chasing**. A doubly linked list is just a chain of these indirections.

### 5.2 Node Linking — How the Chain Is Built

When `push_back(20)` is called on a one-node list:

```
Before:
head → [10 | next=null | prev=null] ← tail

After push_back(20):
head → [10 | next=→ | prev=null]    [20 | next=null | prev=→ ]
                    ↘                ↗                        ↑
                     ────────────────                       tail
```

In code:

```cpp
Node* newNode = new Node(20);      // Allocate new node
newNode->prev = tail;              // newNode looks back at old tail
tail->next = newNode;              // old tail looks forward to newNode
tail = newNode;                    // tail sentinel now points to new node
++listSize;
```

Exactly **4 pointer assignments and 1 allocation**. No element copying, no traversal. The chain grows by one node and a handful of pointers move.

### 5.3 Why Scattered Memory is Fundamental

Scattered memory isn't a flaw — it's the direct price of allocating each node independently, which is exactly the feature that makes O(1) insertion anywhere possible. If nodes sat contiguously like an array, inserting in the middle would force a shift. Scattered layout is the cost of structural flexibility.

This is the core tension between the two structures:

- **Array**: O(1) random access (arithmetic addressing), O(N) middle insertion (physical shifting).
- **Linked List**: O(N) random access (pointer chasing), O(1) middle insertion (pointer relinking, given the node pointer).

---

## 6. Dynamic Allocation Per Node

### 6.1 Why Each Node is Heap-Allocated

1. **Unknown count at compile time**: the list grows and shrinks at runtime, so nodes can't live on the stack — a stack-allocated node would dangle the moment the creating function returned.
2. **Independent lifetimes**: each node is created and destroyed on its own schedule. Heap allocation gives that granular control.
3. **Stable addresses**: unlike a dynamic array (which moves everything to a new buffer on reallocation), a node **never moves** once created. You can hold a `Node*` indefinitely — it stays valid as long as that specific node isn't removed.

### 6.2 new vs malloc for Nodes

Unlike `Vector<T>`, which might use `malloc` + placement `new` to separate allocation from construction, `List<T>` just uses plain `new`:

```cpp
Node* newNode = new Node(value);
// Equivalent to:
// void* mem = operator new(sizeof(Node));  // allocate
// new(mem) Node(value);                     // construct
```

This makes sense because for a node, allocation and construction always happen together — there's no "reserve capacity" concept, and we never want an uninitialized node sitting around. The two-phase split that `malloc` + placement-new offers is unnecessary overhead here.

Destruction mirrors this with plain `delete`:

```cpp
delete nodeToRemove;
// Equivalent to:
// nodeToRemove->~Node();    // calls ~T() on data
// operator delete(nodeToRemove);
```

### 6.3 The Ownership Chain

```
List         head
 │            │
 └────────────→ Node₀ ──next──→ Node₁ ──next──→ Node₂ ──next──→ nullptr
                  ↑                ↑                ↑
                  │    prev        │    prev        │
                nullptr ←──────── ─│←────────────── │←──────────── ...
```

`List` owns `head` and, through it, the whole chain. `tail` is a **non-owning alias** to the last node — it exists purely for O(1) tail access and is never used to free memory.

**Memory leak scenario**: lose the `head` pointer without deleting the chain first (e.g. by overwriting it), and every node becomes unreachable — a leak proportional to N.

**Double-free scenario**: if two `List` objects share the same `head` (a shallow copy), both destructors will try to delete the same nodes — a double-free and heap corruption.

These are exactly the bugs the **Rule of Three** (Section 8) is there to prevent.

---

## 7. Constructors and Object Lifetime

### 7.1 Default Constructor

```cpp
template<typename T>
List<T>::List() : head(nullptr), tail(nullptr), listSize(0) {}
```

The list starts empty — nothing is allocated. This satisfies Invariant 1. Unlike a `Vector<T>`, which usually reserves at least one slot up front, `List<T>` costs zero heap memory until the first element is inserted. Three assignments — O(1).

### 7.2 Initializer List Constructor

```cpp
template<typename T>
List<T>::List(std::initializer_list<T> initList)
    : head(nullptr), tail(nullptr), listSize(0) {
    for (const T& value : initList) {
        push_back(value);
    }
}
```

This enables `List<int> lst = {10, 20, 30, 40};`. It starts empty, then calls `push_back` for each element — one O(1) operation per element, so the whole constructor is O(N).

**Exception safety**: if a `push_back` throws partway through (say `new` runs out of memory), the destructor of the partially-built list still runs and cleans up everything allocated so far. No leaks.

### 7.3 Copy Constructor

```cpp
template<typename T>
List<T>::List(const List& other) : head(nullptr), tail(nullptr), listSize(0) {
    copyFrom(other);
}

template<typename T>
void List<T>::copyFrom(const List& other) {
    Node* current = other.head;
    while (current != nullptr) {
        push_back(current->data);    // Allocate new node, copy data
        current = current->next;
    }
}
```

This makes a **deep copy** — an entirely separate chain of nodes holding the same values in the same order:

```
BEFORE:  other.head → [A]→[B]→[C]→null      (other owns these 3 nodes)

AFTER:   other.head → [A]→[B]→[C]→null      (other still owns its nodes)
         this->head → [A]→[B]→[C]→null      (this owns 3 NEW nodes, different addresses)
```

Modifying one list afterward never touches the other.

**Why a shallow copy would be catastrophic**: if we just wrote `head = other.head; tail = other.tail; listSize = other.listSize;`, both lists would point at the same nodes. Whichever is destroyed first deletes every shared node — the other list is left holding dangling pointers. Any access after that, even just running its own destructor, is undefined behavior (use-after-free).

**Complexity**: O(N) — N allocations, N data copies.

### 7.4 Copy Assignment Operator

```cpp
template<typename T>
List<T>& List<T>::operator=(const List& other) {
    if (this != &other) {       // Self-assignment guard
        clear();                // Destroy and free all existing nodes
        copyFrom(other);        // Deep-copy from other
    }
    return *this;
}
```

A linked list can't reuse its existing nodes for different data the way an array might reuse its buffer — the node *is* the allocation unit. So assignment works in two steps: destroy everything currently held via `clear()` (O(M) for the current size M), then deep-copy from `other` via `copyFrom()` (O(N)). Total: O(M+N) = O(N).

**Self-assignment guard**: without `if (this != &other)`, `clear()` would wipe out the very list `copyFrom` is about to read from.

**Order matters for exception safety**: `clear()` goes first and can't fail (destructors are `noexcept`). If `copyFrom` throws midway, the list ends up in a valid, if incomplete, state — not corrupt, since whatever nodes were already linked in will be cleaned up properly by the destructor later.

### 7.5 Destructor

```cpp
template<typename T>
List<T>::~List() noexcept {
    clear();
}

template<typename T>
void List<T>::clear() {
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;    // Save next before deleting current
        delete current;                // Calls ~Node() → calls ~T() on data
        current = next;                // Advance to saved next
    }
    head = nullptr;
    tail = nullptr;
    listSize = 0;
}
```

**Critical subtlety**: `next` must be saved *before* `delete current` runs. Once `delete current` executes, `current` points at freed memory — reading `current->next` after that is undefined behavior.

**Why `delete` reaches `~T()`**: `delete node` calls `Node::~Node()`. Because `Node` contains `T data` as a member, `~Node()` automatically calls `data.~T()` — C++ destructs every data member when a class is destroyed. No manual cleanup needed.

**`noexcept`**: destructors must never throw. If one does during stack unwinding (while another exception is already in flight), `std::terminate()` gets called immediately. This holds as long as `~T()` itself is `noexcept`, which any well-behaved type should be.

**Complexity**: O(N) — every node is visited exactly once.

---

## 8. Rule of Three

**The Rule of Three**: if a class defines any one of destructor, copy constructor, or copy assignment, it must define all three.

| Function | What It Does | What Happens Without It |
|:---|:---|:---|
| **Destructor** `~List()` | Traverses and `delete`s every node | Memory leak — N nodes allocated, none freed |
| **Copy Constructor** `List(const List&)` | Deep-copies every node | Compiler generates shallow copy → shared nodes → double-free |
| **Copy Assignment** `operator=(const List&)` | Clears self, deep-copies other | Compiler generates shallow assignment → same aliasing bug |

`List<T>` manages heap memory, and the compiler's default copy operations only do a **memberwise copy** — they copy `head` and `tail` as raw pointer values, not the nodes they point to. That gives two `List` objects with identical `head`/`tail`, silently sharing ownership of one chain. When either is destroyed, it frees every node; the other is left with dangling pointers, and touching it afterward — even just letting its destructor run — is undefined behavior. That's exactly why all three functions are hand-written:

```
~List()                → clear() → traverse and delete every node
List(const List&)      → copyFrom(other) → allocate new nodes, copy data
operator=(const List&) → clear() + copyFrom(other)
```

---

## 9. Public API

```cpp
template<typename T>
class List {
public:
    struct Node {
        T data;
        Node* next;
        Node* prev;
        Node(const T& value): data(value), next(nullptr), prev(nullptr) {}
    };

private:
    Node* head;
    Node* tail;
    size_t listSize;
    void copyFrom(const List& other);

public:
    List();
    List(const List& other);
    List& operator=(const List& other);
    ~List() noexcept;

    List(std::initializer_list<T> initList);

    void push_back(const T& value);
    void push_front(const T& value);
    void insert(size_t index, const T& value);
    void remove(size_t index);

    void pop_back();
    void pop_front();

    T& front();
    const T& front() const;

    T& back();
    const T& back() const;

    T& at(size_t index);
    const T& at(size_t index) const;

    [[nodiscard]] bool empty() const noexcept;
    size_t size() const noexcept;
    void clear();
    bool contains(const T& value) const;
    Node* find(const T& value) const;
    void reverse();
};
```

---

## 10. Traversal Optimization

### 10.1 Bidirectional Traversal

Because the list keeps both `head` and `tail`, and every node has both `next` and `prev`, you can start a traversal from whichever end is closer. This is the key optimization behind `at(index)` and `insert(index)`.

A naïve traversal always starts at `head`. Reaching the last element (`at(N-1)`) takes N−1 steps.

The optimized version asks: is the target in the first half of the list, or the second?

- If `index < listSize / 2`: go forward from `head` — at most `⌊N/2⌋` steps.
- If `index ≥ listSize / 2`: go backward from `tail` — at most `⌈N/2⌉` steps.

```cpp
Node* List<T>::getNodeAt(size_t index) const {
    Node* current;
    if (index < listSize / 2) {
        current = head;
        for (size_t i = 0; i < index; ++i)
            current = current->next;
    } else {
        current = tail;
        for (size_t i = listSize - 1; i > index; --i)
            current = current->prev;
    }
    return current;
}
```

### 10.2 The N/2 Optimization — Mathematical Proof

**Claim**: bidirectional traversal cuts the maximum number of steps from N−1 down to ⌊N/2⌋.

**Proof**: let the list have N nodes indexed 0 to N−1, and let `k` be the target index.

**Naïve (forward-only)**:
- Steps from `head` = k
- Maximum over all k ∈ {0,...,N−1}: N−1

**Bidirectional**:
- If k < N/2: steps from `head` = k < N/2
- If k ≥ N/2: steps from `tail` = (N−1) − k ≤ (N−1) − N/2 = N/2 − 1 < N/2
- Maximum over all k: ⌊(N−1)/2⌋

| Method | Worst-case index | Steps |
|:---|:---|:---|
| Forward-only | k = N−1 | N−1 steps |
| Bidirectional | k = ⌊N/2⌋ or ⌈N/2⌉ | ⌊N/2⌋ steps |

**Concrete example for N = 1,000,000**: forward-only worst case is 999,999 pointer dereferences; bidirectional worst case is 499,999 — roughly half the work saved per lookup.

### 10.3 Why O(N/2) is Still O(N)

Despite the real 2× speedup, **O(N/2) = O(N)** in Big-O notation. Here's the formal proof.

**Big-O definition**: f(N) = O(g(N)) if there exist constants c > 0 and N₀ such that f(N) ≤ c × g(N) for all N ≥ N₀.

**Claim**: N/2 = O(N).

**Proof**: let f(N) = N/2 and g(N) = N. We need c and N₀ such that N/2 ≤ c × N for all N ≥ N₀. Choosing c = 1 and N₀ = 1: N/2 ≤ 1 × N holds for every N ≥ 1. ∎

**Intuition**: Big-O captures how growth *scales*, not raw speed. Both N/2 and N grow linearly — double N and both double. The constant factor (1/2) doesn't change the asymptotic class. The optimization is real and worth having (2× faster in practice), but it doesn't change the category. If you genuinely need O(1) access by index, you need a different structure — a linked list's `at(index)` can never beat O(N), no matter how clever the traversal.

---

## 11. Pointer Relinking — The Core Operation

Every insertion and deletion boils down to a sequence of pointer reassignments. Getting one assignment wrong can silently break Invariant 6 and corrupt the list.

### 11.1 Insert at Front (push_front)

**Before** (list: 20 → 30):
```
head → [20 | prev=null | next=→] → [30 | prev=← | next=null] ← tail
```

**Goal**: insert `[10]` before `[20]`.

```
Step 1: Allocate newNode with data=10, next=null, prev=null
Step 2: newNode->next = head         (newNode points forward to old head)
Step 3: head->prev = newNode         (old head points back to newNode)
Step 4: head = newNode               (head sentinel updated)
Step 5: if list was empty: tail = newNode
Step 6: ++listSize
```

**After** (list: 10 → 20 → 30):
```
head → [10 | prev=null | next=→] → [20 | prev=← | next=→] → [30 | prev=← | next=null] ← tail
```

**Pointer assignments**: 4 + 1 allocation. **O(1)**.

### 11.2 Insert at Back (push_back)

**Before** (list: 10 → 20):
```
head → [10 | prev=null | next=→] → [20 | prev=← | next=null] ← tail
```

**Goal**: insert `[30]` after `[20]`.

```
Step 1: Allocate newNode with data=30, next=null, prev=null
Step 2: newNode->prev = tail
Step 3: tail->next = newNode
Step 4: tail = newNode
Step 5: if list was empty: head = newNode
Step 6: ++listSize
```

**Pointer assignments**: 4 + 1 allocation. **O(1)**.

### 11.3 Insert in the Middle

**Before** (list: 10 → 30, inserting 20 between them):
```
head → [10 | prev=null | next=→30] → [30 | prev=←10 | next=null] ← tail
         A                               B
```

**Goal**: insert `[20]` between A and B.

```
Step 1: Allocate newNode (N) with data=20
Step 2: N->prev = A          (newNode looks back at A)
Step 3: N->next = B          (newNode looks forward to B)
Step 4: A->next = N          (A now points at newNode, not B)
Step 5: B->prev = N          (B now points back at newNode, not A)
Step 6: ++listSize
```

**Critical ordering**: Steps 2–3 must happen before Steps 4–5. If `A->next = N` runs before `N->next = B`, you've already lost your only pointer to B before you got the chance to record it. Mnemonic: **set the newcomer's pointers before breaking the old link.**

**After**:
```
head → [10 | prev=null | next=→N] → [20 | prev=←10 | next=→30] → [30 | prev=←N | next=null] ← tail
```

**Pointer assignments**: 4 + 1 allocation, **O(1)** once the predecessor is known. Finding that predecessor is O(N) traversal — so `insert(index, value)` is O(N) overall, even though the relinking step itself is O(1).

### 11.4 Remove at Front (pop_front)

**Before** (list: 10 → 20 → 30):
```
head → [10 | prev=null | next=→20] → [20 | prev=←10 | next=→30] → ...
```

```
Step 1: Node* toDelete = head
Step 2: head = head->next
Step 3: if head != nullptr: head->prev = nullptr
        else: tail = nullptr
Step 4: delete toDelete
Step 5: --listSize
```

**Pointer assignments**: 2–3 + 1 deletion. **O(1)**.

### 11.5 Remove at Back (pop_back)

**Before** (list: 10 → 20 → 30):
```
... → [20 | prev=←10 | next=→30] → [30 | prev=←20 | next=null] ← tail
```

```
Step 1: Node* toDelete = tail
Step 2: tail = tail->prev               (retreat tail — was O(N) in singly linked!)
Step 3: if tail != nullptr: tail->next = nullptr
        else: head = nullptr
Step 4: delete toDelete
Step 5: --listSize
```

**Pointer assignments**: 2–3 + 1 deletion. **O(1)**. The `tail->prev` dereference in Step 2 is the exact thing a singly linked list can't do — there, `pop_back` would have to traverse from `head`. Here, it's one pointer read.

### 11.6 Remove in the Middle

**Before** (list: 10 → 20 → 30, removing 20):
```
[10 | next=→N] → [20 | prev=←10 | next=→30] → [30 | prev=←N | next=null]
  A                       N (to remove)                 B
```

```
Step 1: Node* A = N->prev
Step 2: Node* B = N->next
Step 3: if A != nullptr: A->next = B    else: head = B
Step 4: if B != nullptr: B->prev = A    else: tail = A
Step 5: delete N
Step 6: --listSize
```

**After**:
```
[10 | next=→30] → [30 | prev=←10 | next=null]
```

**Pointer assignments**: 4 + 1 deletion. **O(1)** given the node pointer N.

---

## 12. Function-by-Function Reference

Each entry covers what the function does internally, its best/worst case, and why.

**`push_back(value)`** — allocate a node, link it after `tail` (Section 11.2), update `tail`, increment size. No loops, no work that depends on N. **O(1)** best and worst.

**`push_front(value)`** — same structure as `push_back`, mirrored at the front (Section 11.1). **O(1)** best and worst.

**`insert(index, value)`** — bounds-checks first. If `index == 0` or `index == listSize`, delegates straight to `push_front`/`push_back` (O(1)). Otherwise, traverses bidirectionally to the node at `index` and performs the 4-pointer middle insertion (Section 11.3).
*Proof*: traversal steps S = min(index, listSize − index), maximized at index ≈ N/2 giving ⌊N/2⌋ steps. Since ⌊N/2⌋ = Θ(N), traversal is the dominant cost; the relinking itself is O(1). **Best: O(1)** (at the edges). **Worst: O(N)** (near the middle).

**`remove(index)`** — same structure as `insert`: delegates to `pop_front`/`pop_back` at the edges, otherwise traverses then performs the 4-pointer removal (Section 11.6) and deletes the node. **Best: O(1)**. **Worst: O(N)**, by the identical traversal argument.

**`pop_back()`** — checks for empty, then `tail = tail->prev` (a single dereference, not a traversal), fixes up `next`/`head`, deletes the old tail, decrements size. **O(1)** best and worst — this is the operation that a singly linked list cannot do without traversal.

**`pop_front()`** — mirror of `pop_back` at the front: `head = head->next`, fix up `prev`/`tail`, delete, decrement. **O(1)** best and worst.

**`front()` / `back()`** — return `head->data` / `tail->data` directly after an empty check. **O(1)** — contrast with a list that only tracks `head`, where `back()` would need a full O(N) traversal.

**`at(index)`** — bounds-checks, then uses bidirectional traversal (Section 10.1) to reach the node, then returns its data.
*Proof*: steps = min(index, listSize − 1 − index) ≤ ⌊(N−1)/2⌋ = O(N). **Best: O(1)** (index 0 or listSize−1). **Worst: O(N)** (the middle). Contrast with `Vector::at(index)`, which computes `data + index × sizeof(T)` directly in O(1) regardless of N — there's no equivalent shortcut for a list, since each node must be visited individually.

**`find(value)`** — linear scan from `head`, comparing `data == value` at each step, returning the matching `Node*` or `nullptr`.
*Proof*: the loop runs at most N times, each step O(1), so total is O(N). **Best: O(1)** (head matches). **Worst: O(N)** (no match, or match at the end). Returning a `Node*` rather than an index matters: with the pointer in hand, a later insert/remove at that spot is O(1) pure relinking — an index would force a second O(N) traversal just to relocate the node.

**`contains(value)`** — calls `find` and checks for non-null. Same complexity as `find`: **O(1)** best, **O(N)** worst.

**`reverse()`** — walks from `head`, swapping each node's `next`/`prev`, then swaps `head` and `tail`.
*Proof*: visits each of N nodes once, each visit O(1) (`std::swap` of two pointers), plus one final O(1) swap of the sentinels. Total: N × O(1) + O(1) = **O(N)**, both best and worst — every node must be touched, there's no early exit. Space is O(1): only a couple of traversal variables, no auxiliary structures.

**`clear()`** — walks from `head`, saving `next` before each `delete` (the saved-before-use rule matters here, since deleting first would leave `current` pointing at freed memory), then resets `head`/`tail`/`listSize`.
*Proof*: exactly `listSize` iterations, each O(1) (save + delete + advance). **Best: O(1)** (already empty). **Worst: O(N)**.

**`size()` / `empty()`** — return the stored `listSize` counter directly, or compare it to zero. **O(1)** always — this is exactly why `listSize` is maintained incrementally on every push/pop instead of being recomputed by traversal.

**Copy Constructor** — starts empty, then `push_back`s every element from the source via `copyFrom`. N calls to an O(1) operation: **O(N)**, both best and worst, since the whole source must be visited regardless.

**Copy Assignment** — `clear()`s the current M nodes, then `copyFrom()`s the N nodes of `other`. **Best: O(1)** (self-assignment, immediate return). **Worst: O(M + N) = O(N)**.

**Destructor** — calls `clear()`. **Best: O(1)** (empty list). **Worst: O(N)** (N nodes to free).

---

## 13. Time Complexity Analysis

| Operation | Complexity | Structural Reason |
|:---|:---|:---|
| `push_front` | **O(1)** | `head` pointer gives direct access to front. 4 pointer assignments. |
| `push_back` | **O(1)** | `tail` pointer gives direct access to back. 4 pointer assignments. |
| `pop_front` | **O(1)** | `head->next` gives new head. 3 pointer ops + 1 `delete`. |
| `pop_back` | **O(1)** | `tail->prev` gives new tail — the `prev` pointer's whole purpose. |
| `front` | **O(1)** | Direct dereference of `head->data`. |
| `back` | **O(1)** | Direct dereference of `tail->data`. |
| `at(index)` | **O(N)** | Memory is scattered — must follow the chain node by node. |
| `insert(index)` | **O(N)** | Traversal to find position: O(N). Relinking itself: O(1). |
| `remove(index)` | **O(N)** | Traversal to find node: O(N). Relinking + delete: O(1). |
| `find(value)` | **O(N)** | Linear scan — no index structure, no sorted order assumed. |
| `contains(value)` | **O(N)** | Delegates to `find`. |
| `reverse()` | **O(N)** | Must visit every node once to swap its `next`/`prev`. |
| `clear()` | **O(N)** | Must `delete` every node. |
| `size()` | **O(1)** | Returns stored `listSize` counter. |
| `empty()` | **O(1)** | Compares `listSize == 0`. |
| Copy Constructor | **O(N)** | Allocates N new nodes, copies N data elements. |
| Copy Assignment | **O(N)** | Clears M + copies N = O(M+N) = O(N). |
| Destructor | **O(N)** | Deletes all N nodes. |

---

## 14. Complexity Proofs

### push_front / push_back — O(1)

**Proof**: both perform 1 `new Node(value)` call (O(1) single heap allocation), at most 4 pointer assignments (O(1) each), and 1 increment of `listSize` (O(1)). None of this depends on N — no loop, no traversal, no element copying. The work is bounded by a constant c for every N ≥ 1, which satisfies the Big-O definition with c and N₀ = 1. Total: O(1). ∎

### pop_front / pop_back — O(1)

**Proof for `pop_back`** (the non-trivial case): it performs 1 comparison (`listSize == 0`), 1 pointer read (`tail->prev`), 1–2 pointer assignments, 1 `delete`, and 1 decrement — all O(1). The key step, `tail = tail->prev`, is **a single dereference**, not a traversal: the `prev` pointer on the tail node already holds the address of the second-to-last node directly. No iteration is needed. Total: O(1). ∎

### insert(index, value) — O(N)

**Proof**: let T(N, k) be the time for `insert(k, value)` on a list of size N.

For k = 0 (push_front): T(N, 0) = O(1), no traversal.
For k = N (push_back): T(N, N) = O(1), no traversal.
For 0 < k < N: traversal steps = min(k, N − k) (bidirectional optimization), each step O(1), so traversal cost = O(min(k, N − k)); insertion itself (once the node is found) is O(1).

Worst case: k = ⌊N/2⌋, giving min(⌊N/2⌋, N − ⌊N/2⌋) = ⌊N/2⌋ steps. Since ⌊N/2⌋ = Θ(N): T(N, ⌊N/2⌋) = Θ(N). Therefore the worst-case time complexity of `insert` is Θ(N), which is O(N). ∎

### remove(index) — O(N)

**Proof**: identical structure to `insert`. Traversal to position k costs O(min(k, N − k)), maximized at k = ⌊N/2⌋ = Θ(N). The removal itself (relinking + `delete`) is O(1). Total worst case: O(N). ∎

### at(index) — O(N)

**Proof**: with bidirectional optimization, accessing index k requires k steps forward from `head` if k < N/2, or (N − 1 − k) steps backward from `tail` if k ≥ N/2. So S(k) = min(k, N − 1 − k), and

max S(k) = S(⌊(N−1)/2⌋) = ⌊(N−1)/2⌋ ≈ N/2 for large N.

Since N/2 = O(N) (constants drop out of Big-O), the worst case is O(N). ∎

**Contrast with array**: `Vector::at(k)` computes `data + k × sizeof(T)` — one multiply-and-add, O(1) regardless of k and N.

### find(value) — O(N)

**Proof**: linear search visits nodes in order n₀, n₁, ..., n_{N-1}, stopping at the first match. Best case (match at n₀): 1 comparison, O(1). Worst case (no match, or match at the end): N comparisons, O(N). Average case (uniform random target): roughly (N+1)/2 comparisons, still O(N). Each comparison (`T::operator==`) is assumed O(1). Total worst case: N × O(1) = O(N). ∎

**Could `find` be faster than O(N)?** Only with extra structure: a sorted list still needs O(N) per insertion to keep it sorted, and binary search on a linked list is still O(N) since you can't jump to the midpoint in O(1). A hash map of values → nodes gives O(1) average lookup but costs O(N) extra space. For a plain unsorted doubly linked list, O(N) linear search is the best you can do.

### reverse() — O(N)

**Proof**: the algorithm visits each node exactly once, performing `std::swap(current->next, current->prev)` — a constant-time operation — at each. After the loop, `std::swap(head, tail)` is O(1). Total: N × O(1) + O(1) = O(N).

**Lower bound**: any correct reversal must at least read every node once to place it in the new order — that's Ω(N) unavoidably. So this O(N) algorithm is asymptotically optimal. ∎

### clear() — O(N)

**Proof**: the loop runs exactly `listSize = N` iterations. Each iteration saves `next` (O(1)), deletes the current node (O(1) amortized), and advances (O(1)). Total: N × O(1) = O(N). Resetting `head`, `tail`, `listSize` afterward is O(1). Grand total: O(N). ∎

### Copy Constructor — O(N)

**Proof**: `copyFrom` calls `push_back` once per node in the source. Each `push_back` is O(1), and the source has N nodes. Total: N × O(1) = O(N). ∎

### Destructor — O(N), or O(1) when empty

**Proof**: delegates to `clear()`. If N = 0, the loop never runs: O(1). If N > 0: O(N). ∎

---

## 15. Linked List vs Dynamic Array — When to Choose Which

| Criterion | Choose `List<T>` | Choose `Vector<T>` |
|:---|:---|:---|
| **Dominant operation** | Insertions/deletions at both ends | Random access by index |
| **Iteration pattern** | Sequential (front-to-back or back-to-front) | Random or indexed |
| **Element stability** | Must hold stable pointers/iterators to elements | Pointers can be invalidated (reallocation) |
| **Middle insertion** | Frequent (given a node pointer: O(1)) | Rare (O(N) shifting) |
| **Memory pattern** | Predictable, per-element allocation | Bursty (rare large reallocations) |
| **Cache performance** | Not a concern (or data too large to benefit) | Critical (tight loops, SIMD, prefetch) |
| **Memory overhead** | Acceptable (16 bytes/node extra) | Minimal (`sizeof(T)` per element) |
| **Queue / Deque** | **Yes** — O(1) push/pop at both ends | Requires tricks (circular buffer or `deque`) |
| **Stack** | Yes — O(1) push/pop at one end | Yes — O(1) amortized `push_back`/`pop_back` |
| **Sorted structure** | Poor (insertion sort is O(N²) due to O(N) traversal) | Same, but better cache for sort algorithms |

**The golden rule**: if you ever need O(1) random access (`at(i)`), use a `Vector`. If you need strict O(1) insertion/deletion at both ends and never need random access, use a `List`.

In practice, `Vector<T>` wins for most general-purpose use cases because CPU cache behavior dominates real-world performance. A `Vector` of 1M integers fits in ~4 MB of contiguous memory; a `List` of 1M integers needs ~1M separate heap allocations totaling ~20 MB, with much worse cache behavior on any sequential scan.

`List<T>` wins when:
- You're implementing a **queue** or **deque** (enqueue at back, dequeue at front — both O(1))
- You need **stable iterators** that survive insertions elsewhere (no reallocation ever moves a node)
- You're building a **kernel process list**, **LRU cache**, or **undo/redo stack**
- You perform many **O(1) middle insertions** at an already-known position (e.g., splicing lists)