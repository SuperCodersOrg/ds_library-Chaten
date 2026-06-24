# Dynamic Array (Vector) — Design Document
---

## Table of Contents

1.  [Introduction](#1-introduction)
    - 1.1 [What is a Dynamic Array](#11-what-is-a-dynamic-array)
    - 1.2 [Why Dynamic Arrays Exist](#12-why-dynamic-arrays-exist)
    - 1.3 [Static Arrays vs Dynamic Arrays](#13-static-arrays-vs-dynamic-arrays)
    - 1.4 [Real-World Use Cases](#14-real-world-use-cases)
2.  [Design Evolution](#2-design-evolution)
    - 2.1 [Attempt 1 — Fixed-Size Array](#21-attempt-1--fixed-size-array)
    - 2.2 [Attempt 2 — Allocate on Every Insertion](#22-attempt-2--allocate-on-every-insertion)
    - 2.3 [Attempt 3 — Geometric Growth](#23-attempt-3--geometric-growth)
3.  [Internal Architecture](#3-internal-architecture)
    - 3.1 [Member Variables](#31-member-variables)
    - 3.2 [Ownership Semantics](#32-ownership-semantics)
    - 3.3 [Class Invariants](#33-class-invariants)
4.  [Memory Layout](#4-memory-layout)
5.  [Heap Allocation Fundamentals](#5-heap-allocation-fundamentals)
    - 5.1 [Stack vs Heap](#51-stack-vs-heap)
    - 5.2 [C Memory Functions](#52-c-memory-functions)
    - 5.3 [Why This Implementation Uses malloc/free](#53-why-this-implementation-uses-mallocfree)
6.  [Allocation vs Construction](#6-allocation-vs-construction)
    - 6.1 [The Two-Phase Object Model](#61-the-two-phase-object-model)
    - 6.2 [Why the Distinction Matters](#62-why-the-distinction-matters)
7.  [Placement New](#7-placement-new)
    - 7.1 [The Three Forms of new](#71-the-three-forms-of-new)
    - 7.2 [Placement New in This Implementation](#72-placement-new-in-this-implementation)
    - 7.3 [Destruction After Placement New](#73-destruction-after-placement-new)
    - 7.4 [Why malloc Alone is Unsafe for Non-Trivial Types](#74-why-malloc-alone-is-unsafe-for-non-trivial-types)
8.  [Constructors and Object Lifetime](#8-constructors-and-object-lifetime)
    - 8.1 [Default Constructor](#81-default-constructor)
    - 8.2 [Initializer List Constructor](#82-initializer-list-constructor)
    - 8.3 [Copy Constructor](#83-copy-constructor)
    - 8.4 [Copy Assignment Operator](#84-copy-assignment-operator)
    - 8.5 [Destructor](#85-destructor)
9.  [Rule of Three](#9-rule-of-three)
10. [Growth Strategy Analysis](#10-growth-strategy-analysis)
    - 10.1 [Growth Factor Comparison](#101-growth-factor-comparison)
    - 10.2 [Memory Waste vs Reallocation Count](#102-memory-waste-vs-reallocation-count)
    - 10.3 [Why This Implementation Uses 2x Growth](#103-why-this-implementation-uses-2x-growth)
11. [Mathematical Growth Factor Proof](#11-mathematical-growth-factor-proof)
    - 11.1 [Reallocation Count Formula](#111-reallocation-count-formula)
    - 11.2 [Numerical Comparison for N = 1,000,000](#112-numerical-comparison-for-n--1000000)
    - 11.3 [Memory Reuse Theorem](#113-memory-reuse-theorem)
12. [Amortized Analysis of push_back](#12-amortized-analysis-of-push_back)
13. [Reallocation Algorithm](#13-reallocation-algorithm)
    - 13.1 [Step-by-Step Process](#131-step-by-step-process)
    - 13.2 [Trivial vs Non-Trivial Type Handling](#132-trivial-vs-non-trivial-type-handling)
14. [Function-by-Function Deep Dive](#14-function-by-function-deep-dive)
    - 14.1 [push_back](#141-push_back)
    - 14.2 [pop_back](#142-pop_back)
    - 14.3 [insert](#143-insert)
    - 14.4 [remove](#144-remove)
    - 14.5 [clear](#145-clear)
    - 14.6 [reserve](#146-reserve)
    - 14.7 [shrink_to_fit](#147-shrink_to_fit)
    - 14.8 [operator\[\]](#148-operator)
    - 14.9 [at](#149-at)
    - 14.10 [front](#1410-front)
    - 14.11 [back](#1411-back)
    - 14.12 [find](#1412-find)
    - 14.13 [contains](#1413-contains)
    - 14.14 [size / capacity / empty](#1414-size--capacity--empty)
    - 14.15 [Copy Constructor](#1415-copy-constructor)
    - 14.16 [Copy Assignment Operator](#1416-copy-assignment-operator)
    - 14.17 [Destructor](#1417-destructor)
15. [Complexity Proofs](#15-complexity-proofs)

---

## 1. Introduction

### 1.1 What is a Dynamic Array

A dynamic array is a contiguous, resizable sequence container that manages its own heap-allocated memory buffer. Unlike fixed-size arrays whose dimensions must be known at compile time (or at the point of stack allocation), a dynamic array grows and shrinks at runtime, automatically reallocating its internal buffer when the current capacity is exhausted.

At the hardware level, a dynamic array is nothing more than a pointer to a heap-allocated block of memory, combined with two pieces of metadata — the number of elements currently stored (`size`) and the total number of elements the buffer can hold before a reallocation is required (`capacity`). This simplicity is precisely what makes the dynamic array the most efficient general-purpose sequence container.

This project provides a custom, from-scratch implementation — `Vector<T>` — built using raw `malloc`/`free` memory management, placement `new` for object construction, and explicit destructor calls for object teardown. It is designed as an educational deep-dive into the systems-level engineering behind dynamic arrays.

### 1.2 Why Dynamic Arrays Exist

The fundamental tension in systems programming is between **static predictability** and **runtime flexibility**:

| Constraint | Static Array | Dynamic Array |
|:---|:---|:---|
| Size known at compile time | **Required** | Not required |
| Memory location | Stack (or static segment) | Heap |
| Resizable | No | Yes |
| Bounds checking | Manual | Optional (via `at()`) |
| Lifetime | Scope-bound | Programmer-controlled |

In real systems, the size of data is almost never known at compile time. A web server does not know how many connections it will handle. A compiler does not know how many tokens are in a source file. A game engine does not know how many particles are on screen. Dynamic arrays exist to solve this fundamental mismatch between the rigidity of static memory and the fluidity of runtime data.

### 1.3 Static Arrays vs Dynamic Arrays

**Static arrays** (C-style `T arr[N]` or `std::array<T, N>`) live on the stack. Their size `N` is baked into the type at compile time. They cannot grow, they cannot shrink, and if `N` is too large, they overflow the stack (typically 1–8 MB).

**Dynamic arrays** separate the control structure from the data. The `Vector<T>` object itself — containing just a pointer, a size, and a capacity — occupies a fixed 24 bytes on the stack (on a 64-bit system). The actual element storage lives on the heap, where gigabytes of memory are available. This indirection is the price paid for runtime flexibility.

```
Static Array (Stack):
┌──────────────────────────────────────┐
│ a₀ │ a₁ │ a₂ │ ... │ aₙ₋₁          │  ← Entire array on the stack
└──────────────────────────────────────┘
  Size fixed at compile time. Cannot grow.

Dynamic Array (Stack + Heap):
  Stack                          Heap
┌─────────────┐         ┌──────────────────────────────┐
│ data ────────│────────→│ a₀ │ a₁ │ a₂ │ ... │  -  │  │
│ size: 3      │         └──────────────────────────────┘
│ capacity: 5  │           Elements can grow at runtime.
└─────────────┘
  24 bytes fixed.
```

### 1.4 Real-World Use Cases

Dynamic arrays are the **default data structure** in nearly every systems programming context:

- **Compilers**: Token streams, AST node children, symbol tables, instruction buffers
- **Game Engines**: Entity lists, vertex buffers, particle systems, render queues
- **Networking**: Packet buffers, connection pools, HTTP header lists
- **Databases**: Row buffers, query result sets, B-tree page contents
- **Operating Systems**: Process tables, page frame lists, I/O request queues
- **Machine Learning**: Tensor storage (flattened multi-dimensional arrays)

The contiguous memory layout of dynamic arrays makes them the fastest container for sequential access due to CPU cache locality — elements are stored side-by-side in memory, allowing the CPU to prefetch upcoming elements into its cache hierarchy.

---

## 2. Design Evolution

The design of `Vector<T>` did not begin with geometric growth and placement `new`. It evolved through a series of failed attempts, each exposing a fundamental limitation that the next design corrected. Understanding this evolution is critical to understanding *why* the final design exists.

### 2.1 Attempt 1 — Fixed-Size Array

**Design**: Allocate a fixed-size buffer at construction time.

```cpp
template<typename T>
class Vector {
    T data[1000];      // Fixed capacity of 1000
    size_t currentSize;
};
```

**Problems**:

| Problem | Impact | Severity |
|:---|:---|:---|
| **Memory waste** | If only 3 elements are stored, 997 slots are wasted | High |
| **Capacity limitation** | Cannot store more than 1000 elements | Critical |
| **Stack overflow** | For large `T` (e.g., 1 KB structs), `1000 × 1024 = 1 MB` overflows the stack | Critical |
| **Inflexible** | Every use case must share the same hardcoded limit | High |

**Verdict**: This approach fundamentally conflates "how much memory is allocated" with "how much data exists." It provides no mechanism for the data structure to adapt to its actual workload.

### 2.2 Attempt 2 — Allocate on Every Insertion

**Design**: Use `malloc` to allocate exactly `size` bytes. On every `push_back`, allocate `size + 1` bytes, copy all elements, free the old block.

```cpp
void push_back(const T& value) {
    T* newData = (T*)malloc((currentSize + 1) * sizeof(T));
    memcpy(newData, data, currentSize * sizeof(T));
    free(data);
    data = newData;
    data[currentSize] = value;
    ++currentSize;
}
```

**Problems**:

This design incurs a **copy of all existing elements on every single insertion**. The total number of element copies for `N` insertions is:

```
Total copies = 0 + 1 + 2 + 3 + ... + (N-1) = N(N-1)/2 = O(N²)
```

For `N = 1,000,000`:

```
Total copies = 1,000,000 × 999,999 / 2 ≈ 5 × 10¹¹ copies
```

Additionally, every `push_back` triggers a `malloc`/`free` pair. Heap allocation is a **system call** (ultimately calling `mmap` or `brk` on Linux, `HeapAlloc` on Windows). Each call costs hundreds to thousands of CPU cycles. For 1 million insertions, this means 1 million system calls — an unacceptable overhead.

**Verdict**: O(N²) total work for N insertions makes this design unusable for any serious workload.

### 2.3 Attempt 3 — Geometric Growth

**Design**: Maintain a `capacity` that is always ≥ `size`. When `size == capacity`, allocate a new buffer that is `2 × capacity` (or some other constant multiple). This is the design used in our `Vector<T>` implementation.

```cpp
void resize() {
    currentCapacity *= 2;
    T* newData = static_cast<T*>(malloc(currentCapacity * sizeof(T)));
    // ... move elements, destroy old objects, free old memory ...
    data = newData;
}
```

**Why this works**: The total number of element copies across all reallocations for `N` insertions is:

```
Total copies = 1 + 2 + 4 + 8 + ... + N = 2N - 1 = O(N)
```

Amortized over N insertions:

```
Amortized cost per insertion = O(N) / N = O(1)
```

The geometric growth strategy converts O(N²) total work into O(N) total work by performing reallocations **exponentially less frequently** as the vector grows. This is proven rigorously in [Section 12](#12-amortized-analysis-of-push_back).

---

## 3. Internal Architecture

### 3.1 Member Variables

The `Vector<T>` class contains exactly three private data members:

```cpp
template<typename T>
class Vector {
private:
    T* data;                // Pointer to heap-allocated buffer
    size_t currentSize;     // Number of live, constructed elements
    size_t currentCapacity; // Total number of T-sized slots in the buffer
};
```

**`T* data`**

A raw pointer to the beginning of a heap-allocated memory block obtained via `malloc`. This pointer represents **sole ownership** of the memory block. The block has space for `currentCapacity` objects of type `T`, but only the first `currentSize` slots contain live, constructed objects.

The pointer `data` is the **only** link between the `Vector` object (which lives on the stack or as part of another object) and the element storage (which lives on the heap). If this pointer is lost or overwritten without first freeing the memory, a **memory leak** occurs.

**`size_t currentSize`**

The number of elements that have been constructed in the buffer using placement `new`. This count represents the "logical" size of the vector — the number of elements visible to the user. Only indices `[0, currentSize)` contain valid, live objects.

`size_t` is an unsigned integer type guaranteed to be large enough to represent the size of any object in memory. On a 64-bit system, it is 8 bytes, allowing a maximum value of `2⁶⁴ - 1` ≈ `1.8 × 10¹⁹`.

**`size_t currentCapacity`**

The total number of `T`-sized slots in the allocated buffer. This is the "physical" capacity — how many elements could be stored before a reallocation is needed. The invariant `currentSize ≤ currentCapacity` must always hold.

### 3.2 Ownership Semantics

`Vector<T>` follows the **exclusive ownership model**:

1. The `Vector` object **owns** the memory pointed to by `data`.
2. No other object or pointer shares ownership of this memory.
3. The owner is responsible for:
   - **Constructing** objects in the buffer (via placement `new`)
   - **Destroying** objects in the buffer (via explicit destructor calls)
   - **Freeing** the buffer itself (via `free`)

This ownership model is enforced through the **Rule of Three** ([Section 9](#9-rule-of-three)): the destructor frees the memory, the copy constructor and copy assignment operator create **independent deep copies** so that two `Vector` objects never share the same underlying buffer.

### 3.3 Class Invariants

The following invariants must hold at all times after construction and between any two public method calls:

```
Invariant 1:  0 ≤ currentSize ≤ currentCapacity
Invariant 2:  currentCapacity ≥ 1  (always at least 1 slot allocated)
Invariant 3:  data ≠ nullptr  (buffer is always allocated)
Invariant 4:  Elements at indices [0, currentSize) are live, constructed objects
Invariant 5:  Slots at indices [currentSize, currentCapacity) are raw memory (no live objects)
```

If any invariant is violated, the behavior of the `Vector` is undefined. For example, if `currentSize > currentCapacity`, accessing `data[currentSize - 1]` would read beyond the allocated buffer — a buffer overread.

---

## 4. Memory Layout

<p align="center">
<img src="../Memory-Diagrams/DynamicArray-Memory Design.jpeg" width="650">
</p>

The diagram above illustrates the complete memory architecture of our `Vector<T>` implementation. Here is a detailed breakdown of every component shown:

**The `data` Pointer**

The `data` member is a raw pointer variable that lives inside the `Vector<T>` object on the stack. It stores the address of the first byte of the heap-allocated array. This pointer is the sole connection between the stack-resident control structure and the heap-resident element storage. Every element access — whether through `operator[]`, `at()`, `front()`, or `back()` — is ultimately a dereference of this pointer with an offset.

**`currentSize`**

Stored as a `size_t` inside the `Vector<T>` object on the stack, `currentSize` represents the number of elements that have been constructed in the buffer. In the diagram, when `size = 5`, elements at indices `[0]` through `[4]` (labelled `a₀` through `a₄`) are live, fully-constructed objects. These are the elements that the user can interact with. The `currentSize` field is incremented by `push_back` and `insert`, and decremented by `pop_back` and `remove`.

**`currentCapacity`**

Also stored as a `size_t` on the stack, `currentCapacity` represents the total number of `T`-sized slots in the heap buffer. In the diagram, when `capacity = 8`, the buffer has room for 8 elements total. This value is only changed during reallocation (when it doubles) or when `reserve` / `shrink_to_fit` are called.

**Occupied Slots**

The shaded/hatched slots in the diagram (indices `[0]` through `[4]`) represent occupied, live objects. These slots contain valid `T` objects that were constructed using placement `new`. Their destructors must be called before the buffer is freed. Every occupied slot is addressable via `data[i]` for `0 ≤ i < currentSize`.

**Unused Slots**

The empty slots in the diagram (indices `[5]` through `[7]`) represent allocated but uninitialized raw memory. No `T` object exists at these addresses — they are simply reserved bytes. Writing to these slots via `operator=` without first constructing an object using placement `new` would be undefined behavior for non-trivial types. These slots exist to absorb future `push_back` calls without triggering a reallocation.

**Ownership Relationship**

The `Vector<T>` object has **exclusive ownership** of the heap buffer. The arrow from `data` to the heap array in the diagram represents this ownership. The `Vector` is solely responsible for:
- Allocating the buffer (`malloc`)
- Constructing objects in it (placement `new`)
- Destroying objects in it (explicit `~T()` calls)
- Freeing the buffer (`free`)

No other object or pointer shares this responsibility. The copy constructor and copy assignment operator create entirely independent buffers, ensuring that two `Vector` objects never co-own the same memory.

**Stack Object vs Heap Memory**

The `Vector<T>` object itself — containing `data`, `currentSize`, and `currentCapacity` — occupies exactly 24 bytes on the stack (on a 64-bit system: 8 bytes for the pointer + 8 bytes for size + 8 bytes for capacity). This is a fixed cost regardless of how many elements are stored. The actual element storage lives on the heap, where the buffer can grow to megabytes or gigabytes as needed. The diagram shows this split clearly: the boxed structure on the left is the stack-resident object, and the array on the right is the heap-resident buffer.

When `push_back` is called and `size == capacity` (the buffer is full, as shown in the middle section of the diagram), the `resize()` function allocates a new, larger buffer on the heap (double the capacity), transfers all elements to it, destroys the old objects, frees the old buffer, and updates the `data` pointer to point to the new buffer. After resize, the `Vector` object on the stack still occupies 24 bytes, but its `data` pointer now points to a different, larger heap allocation — as shown in the bottom section of the diagram.

---

## 5. Heap Allocation Fundamentals

### 5.1 Stack vs Heap

Modern programs have two primary regions of dynamic memory:

```
┌──────────────────────────────────────────────────┐
│                    STACK                          │
│  • Automatic lifetime (scope-based)              │
│  • LIFO allocation/deallocation                  │
│  • Extremely fast (single pointer adjustment)    │
│  • Limited size (typically 1–8 MB)               │
│  • No fragmentation                              │
│  ↓ grows downward                                │
├──────────────────────────────────────────────────┤
│                     ...                          │
├──────────────────────────────────────────────────┤
│  ↑ grows upward                                  │
│                    HEAP                           │
│  • Manual lifetime (programmer-controlled)       │
│  • Arbitrary allocation/deallocation order        │
│  • Slower (bookkeeping, fragmentation)           │
│  • Large (limited by virtual memory, GBs)        │
│  • Subject to fragmentation                      │
└──────────────────────────────────────────────────┘
```

`Vector<T>` **must** use heap memory because:

1. The element buffer size is not known at compile time
2. The buffer must persist across function calls (if the vector is returned or stored)
3. The buffer may need to grow to megabytes or gigabytes — far exceeding stack limits
4. The buffer must be relocatable (reallocated to a different address when growing)

The `Vector<T>` object itself (24 bytes) may live on the stack, but the element buffer it points to always lives on the heap.

### 5.2 C Memory Functions

This implementation uses the C memory allocation functions. Here is a comprehensive comparison:

| Function | Prototype | Behavior | Initializes Memory? | Constructs Objects? |
|:---|:---|:---|:---|:---|
| `malloc` | `void* malloc(size_t size)` | Allocates `size` bytes of raw memory | **No** | **No** |
| `calloc` | `void* calloc(size_t count, size_t size)` | Allocates `count × size` bytes, zero-initialized | **Yes** (to zero) | **No** |
| `realloc` | `void* realloc(void* ptr, size_t size)` | Resizes an existing block (may move it) | **No** (new bytes) | **No** |
| `free` | `void free(void* ptr)` | Returns memory to the allocator | N/A | Does **not** call destructors |

**`malloc(size_t size)`**

Returns a pointer to `size` bytes of uninitialized memory, or `nullptr` on failure. The returned memory is suitably aligned for any fundamental type. This is the function our `Vector<T>` uses:

```cpp
data = static_cast<T*>(malloc(currentCapacity * sizeof(T)));
```

The `static_cast<T*>` is required because `malloc` returns `void*` — C++ does not allow implicit conversion from `void*` to a typed pointer (unlike C).

**Why not `calloc`?**

Zero-initialization is wasted work for our dynamic array. The slots will be initialized via placement `new` with the correct constructor, not with zero bytes. Furthermore, zero-initialized bytes are not necessarily a valid object representation for non-trivial types (a zero-initialized `std::string` is not an empty string — it is undefined behavior).

**Why not `realloc`?**

`realloc` may move the memory block, silently `memcpy`-ing the contents to a new location. For trivially copyable types (`int`, `double`, POD structs), this is safe. For non-trivial types (`std::string`, any class with pointers), `memcpy` creates **bitwise copies** that bypass copy/move constructors. This leaves the old objects in an inconsistent state (e.g., two `std::string` objects sharing the same internal buffer, leading to a **double-free** when both are destroyed).

Our implementation avoids `realloc` entirely. Instead, `resize()` manually allocates a new buffer, moves/copies elements using proper C++ semantics, destroys old objects, and frees the old buffer.

### 5.3 Why This Implementation Uses malloc/free

The choice of `malloc`/`free` is deliberate:

1. **Separation of concerns**: `malloc` gives raw memory. Placement `new` gives constructed objects. These are independent operations that must be invocable independently.
2. **No constructor calls on allocation**: `malloc` does not call any constructor. This is required because the buffer contains `capacity` slots, but only `size` of them should have live objects.
3. **Compatibility with `memcpy`**: For trivially copyable types, `std::memcpy` can be used to bulk-copy elements between buffers — a significant performance optimization. This is safe because `malloc` returns raw bytes, not constructed objects.
4. **Explicit control**: Using `malloc`/`free` makes the memory management strategy visible and auditable. There is no hidden constructor or destructor call.

---

## 6. Allocation vs Construction

### 6.1 The Two-Phase Object Model

In C++, creating an object involves **two distinct phases**:

**Phase 1: Memory Allocation** — Obtaining a block of raw bytes from the heap (or stack).

**Phase 2: Object Construction** — Initializing those bytes into a valid object state by running a constructor.

These phases are **independent**. You can allocate memory without constructing an object in it. You can construct an object in memory that was already allocated (via placement `new`). This separation is the foundation of how our `Vector<T>` manages memory.

```cpp
// Phase 1: Allocate raw memory for 10 ints (no objects exist yet)
int* buffer = static_cast<int*>(malloc(10 * sizeof(int)));

// Phase 2: Construct objects in the allocated memory
for (int i = 0; i < 5; ++i) {
    new(&buffer[i]) int(i * 10);  // Placement new: construct int at &buffer[i]
}

// buffer[0..4] are live int objects (values: 0, 10, 20, 30, 40)
// buffer[5..9] are raw, uninitialized memory (NOT int objects)
```

### 6.2 Why the Distinction Matters

Consider a `Vector<std::string>` with `size = 3` and `capacity = 8`:

```
Heap buffer (8 slots of sizeof(std::string) bytes each):
┌──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┐
│ "hello"  │ "world"  │ "foo"    │ xxxxxxxx │ xxxxxxxx │ xxxxxxxx │ xxxxxxxx │ xxxxxxxx │
└──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┘
  ↑── live std::string objects ──↑  ↑── raw bytes (NOT std::string objects) ──────────↑
  [0]         [1]         [2]        [3]        [4]        [5]        [6]        [7]
```

- Slots `[0]`, `[1]`, `[2]`: Valid `std::string` objects constructed via placement `new`. Their destructors **must** be called before the buffer is freed.
- Slots `[3]` through `[7]`: Raw, uninitialized bytes. **No** `std::string` exists here. Calling a destructor on these slots would be **undefined behavior** (attempting to destroy an object that was never constructed).

If we used `new std::string[8]` instead of `malloc`, all 8 slots would be default-constructed as empty strings — wasting time on constructor calls for slots that may never be used. If we then used `delete[]`, all 8 destructors would run, including for slots that were later overwritten or never properly maintained. The `malloc` + placement `new` approach gives us precise control over which slots contain live objects and which are raw memory.

---

## 7. Placement New

### 7.1 The Three Forms of new

C++ has three distinct forms of `new`, and confusing them is a common source of bugs:

**Form 1: `new` expression (allocating new)**
```cpp
T* p = new T(args...);
```
This does two things: (1) calls `operator new(sizeof(T))` to allocate memory, (2) calls `T(args...)` to construct the object. `delete p` reverses both steps.

**Form 2: `operator new` (raw allocation)**
```cpp
void* mem = operator new(sizeof(T));
```
This only allocates `sizeof(T)` bytes of raw memory. No constructor is called. It is essentially `malloc` with a `std::bad_alloc` exception on failure instead of returning `nullptr`.

**Form 3: Placement `new` (construct in existing memory)**
```cpp
void* mem = malloc(sizeof(T));  // Memory already allocated
T* p = new(mem) T(args...);    // Construct T at address 'mem'
```
This does **not** allocate memory. It constructs a `T` object at the address `mem` using the constructor `T(args...)`. The address `mem` must point to a region of memory that is (a) large enough for `sizeof(T)` bytes, (b) properly aligned for type `T`, and (c) not currently occupied by a live object of a type with a non-trivial destructor.

### 7.2 Placement New in This Implementation

Every element insertion in our `Vector<T>` uses placement `new`:

```cpp
// From push_back:
new(&data[currentSize]) T(value);    // Copy-construct 'value' at data[currentSize]
++currentSize;

// From insert (tail construction during shift):
new(&data[currentSize]) T(std::move(data[currentSize - 1]));

// From resize (moving elements to new buffer):
new(&newData[i]) T(std::move_if_noexcept(data[i]));
```

The syntax `new(&data[i]) T(value)` means:

1. Take the address `&data[i]` — a pointer to `sizeof(T)` bytes of raw memory inside the buffer
2. Invoke `T`'s copy constructor with argument `value` at that address
3. After this call, `data[i]` is a live, fully-constructed `T` object

This is the **only correct way** to construct objects in a pre-allocated buffer. Directly writing `data[i] = value` would invoke `T::operator=` — which assumes a live object already exists at `data[i]`. If `data[i]` is raw memory, this is undefined behavior (calling a member function on a non-existent object).

### 7.3 Destruction After Placement New

Objects created with placement `new` cannot be destroyed with `delete`. The `delete` operator calls both the destructor and `operator delete` (freeing memory). But placement `new` did not allocate memory — so there is nothing for `operator delete` to free. Calling `delete` would attempt to free a pointer into the middle of a `malloc`-ed buffer, causing heap corruption.

Instead, destructors must be called **explicitly**:

```cpp
// Destroy the object at data[i], but do NOT free the memory
data[i].~T();
```

This is one of the only situations in C++ where explicit destructor calls are correct and necessary. Our implementation uses this pattern throughout:

```cpp
// Destructor: destroy all live elements, then free the buffer
if constexpr (!std::is_trivially_destructible<T>::value) {
    for (size_t i = 0; i < currentSize; ++i) {
        data[i].~T();
    }
}
free(data);  // Free the buffer (does NOT call destructors)
```

The `if constexpr` check is a compile-time optimization: for trivially destructible types (like `int`, `double`, POD structs), the destructor is a no-op. Skipping the loop entirely avoids N function calls that do nothing.

### 7.4 Why malloc Alone is Unsafe for Non-Trivial Types

`malloc` returns raw bytes. It does not call constructors. For trivial types like `int`, raw bytes with any value are a valid object representation — you can read and write them freely. But for non-trivial types, raw bytes are **not** a valid object:

```cpp
// UNSAFE: std::string has internal pointers, allocations, SSO buffer
std::string* buf = static_cast<std::string*>(malloc(3 * sizeof(std::string)));

// buf[0], buf[1], buf[2] are NOT std::string objects.
// Their internal pointers are garbage. Their vtable pointers (if any) are garbage.

buf[0] = "hello";   // UNDEFINED BEHAVIOR: operator= called on a non-existent object
                     // Tries to free a garbage internal pointer → crash or corruption

// CORRECT approach:
new(&buf[0]) std::string("hello");  // Construct a valid std::string at buf[0]
```

This is why our `Vector<T>` **always** uses placement `new` to construct elements and **never** writes to uninitialized slots via assignment.

---

## 8. Constructors and Object Lifetime

### 8.1 Default Constructor

```cpp
template<typename T>
Vector<T>::Vector() {
    currentSize = 0;
    currentCapacity = 1;
    data = static_cast<T*>(malloc(currentCapacity * sizeof(T)));
    if (data == nullptr) {
        throw std::bad_alloc();
    }
}
```

**Design decisions**:

- **Initial capacity = 1**: Allocates a minimal buffer of 1 slot. This avoids the need for `nullptr` checks throughout the codebase (invariant 3: `data ≠ nullptr`). The cost of allocating 1 slot is negligible compared to the complexity of handling `nullptr` everywhere.
- **`std::bad_alloc` on failure**: `malloc` returns `nullptr` on allocation failure. We convert this to a C++ exception for RAII compatibility (the caller can catch `std::bad_alloc`).
- **No elements constructed**: `currentSize = 0` means no placement `new` calls. The buffer contains raw memory — no `T` objects exist.

### 8.2 Initializer List Constructor

```cpp
template<typename T>
Vector<T>::Vector(std::initializer_list<T> initList) {
    currentSize = initList.size();
    currentCapacity = currentSize > 0 ? currentSize : 1;
    data = static_cast<T*>(malloc(currentCapacity * sizeof(T)));
    if (data == nullptr) throw std::bad_alloc();

    if constexpr (std::is_trivially_copyable<T>::value) {
        std::memcpy(data, initList.begin(), currentSize * sizeof(T));
    } else {
        size_t constructedCount = 0;
        try {
            for (const T& value : initList) {
                new(&data[constructedCount]) T(value);
                constructedCount++;
            }
        } catch (...) {
            for (size_t i = 0; i < constructedCount; i++) {
                data[i].~T();
            }
            free(data);
            throw;
        }
    }
}
```

This constructor enables brace initialization:

```cpp
Vector<int> v = {10, 20, 30, 40, 50};
```

**Key design point**: The capacity is set exactly to the initializer list size (no extra slack). This avoids wasting memory for vectors whose size is known at construction.

The `if constexpr` branch dispatches at compile time: for trivially copyable types (POD types), `std::memcpy` bulk-copies all elements in a single operation — significantly faster than N individual copy-constructor calls. For non-trivial types, each element is individually copy-constructed via placement `new`, with full exception safety (if any copy constructor throws, all previously constructed elements are destroyed before re-throwing).

### 8.3 Copy Constructor

```cpp
template<typename T>
Vector<T>::Vector(const Vector& other) {
    currentSize = other.currentSize;
    currentCapacity = currentSize > 0 ? currentSize : 1;
    data = static_cast<T*>(malloc(currentCapacity * sizeof(T)));
    if (data == nullptr) throw std::bad_alloc();

    if constexpr (std::is_trivially_copyable<T>::value) {
        std::memcpy(data, other.data, currentSize * sizeof(T));
    } else {
        size_t constructedCount = 0;
        try {
            for (size_t i = 0; i < currentSize; ++i) {
                new(&data[i]) T(other.data[i]);
                constructedCount++;
            }
        } catch (...) {
            for (size_t i = 0; i < constructedCount; i++) {
                data[i].~T();
            }
            free(data);
            throw;
        }
    }
}
```

The copy constructor performs a **deep copy**: it allocates a completely independent buffer and copies each element from the source. After the copy, the two `Vector` objects share no state — modifying one does not affect the other.

```
BEFORE copy:
  src.data ──→ [A][B][C]     (src owns this buffer)

AFTER copy: Vector<T> dst(src);
  src.data ──→ [A][B][C]     (src still owns this buffer)
  dst.data ──→ [A][B][C]     (dst owns a NEW, independent buffer)
```

A **shallow copy** (memberwise copy of the pointer) would make both vectors point to the same buffer. When one vector is destroyed and frees the buffer, the other vector's `data` pointer becomes dangling — leading to use-after-free and double-free crashes. Deep copy is more expensive (O(N) per copy) but is the **only correct** strategy for a value-semantic container that owns heap memory.

### 8.4 Copy Assignment Operator

```cpp
template<typename T>
Vector<T>& Vector<T>::operator=(const Vector& other) {
    if (this != &other) {
        if (currentCapacity >= other.currentSize) {
            // Reuse existing buffer (no reallocation needed)
            // Destroy old elements, then copy-construct new elements
        } else {
            // Allocate new buffer, copy elements, destroy old, swap
        }
    }
    return *this;
}
```

The copy assignment operator is more complex than the copy constructor because the target `Vector` **already has state** that must be cleaned up. The implementation handles two cases:

**Case 1: Existing buffer is large enough** (`currentCapacity >= other.currentSize`)

No reallocation needed. Destroy existing elements, then copy-construct new elements in the existing buffer. This avoids a `malloc`/`free` round-trip.

**Case 2: Existing buffer is too small**

Allocate a new buffer, copy elements into it. Only then destroy old elements and free the old buffer. This order is critical for **exception safety**: if the copy into the new buffer throws, the old buffer is still intact and the `Vector` is unchanged.

**Self-assignment check**: `if (this != &other)` prevents `v = v` from destroying its own data before copying it. Without this check, the vector would destroy its elements, then try to copy from destroyed elements — undefined behavior.

### 8.5 Destructor

```cpp
template<typename T>
Vector<T>::~Vector() noexcept {
    if (data != nullptr) {
        if constexpr (!std::is_trivially_destructible<T>::value) {
            for (size_t i = 0; i < currentSize; ++i) {
                data[i].~T();
            }
        }
        free(data);
        data = nullptr;
        currentSize = 0;
        currentCapacity = 0;
    }
}
```

The destructor performs two cleanup operations in strict order:

1. **Destroy all live elements** (indices `[0, currentSize)`) by calling their destructors
2. **Free the buffer** by calling `free(data)`

The order is critical: elements must be destroyed **before** the memory is freed. Freeing the memory first would make the destructor calls access freed memory (undefined behavior).

The `noexcept` specifier guarantees that the destructor does not throw exceptions. This is essential for safe stack unwinding during exception propagation and for `std::move_if_noexcept` to choose move operations over copies during reallocation.

---

## 9. Rule of Three

The **Rule of Three** is a C++ guideline that states: if a class defines any one of the following special member functions, it should define all three:

| Function | Responsibility | Why It's Needed |
|:---|:---|:---|
| **Destructor** `~Vector()` | Free owned memory, destroy objects | Without it, memory leaks when the vector goes out of scope |
| **Copy Constructor** `Vector(const Vector&)` | Create independent deep copy | Without it, compiler generates a shallow (memberwise) copy → aliasing, double-free |
| **Copy Assignment** `operator=(const Vector&)` | Replace contents with deep copy | Without it, compiler generates shallow assignment → same aliasing bugs |

The logic is straightforward: if a class manages a resource (heap memory, file handle, network socket), then the compiler-generated default for any of these three functions is almost certainly wrong. The default copy constructor and copy assignment both perform **memberwise copy** — they copy the pointer value, not the pointed-to data. This creates two objects that believe they own the same resource, leading to double-free and use-after-free bugs.

Our `Vector<T>` implements all three:

```
~Vector()                 → Destroys elements + frees buffer
Vector(const Vector&)     → Allocates new buffer + deep-copies elements
operator=(const Vector&)  → Destroys old + allocates new + deep-copies elements
```

The **Rule of Five** extends this to include the move constructor and move assignment operator. Our current implementation does not provide move semantics at the container level, but internally uses `std::move_if_noexcept` during reallocation to transfer elements efficiently.

---

## 10. Growth Strategy Analysis

### 10.1 Growth Factor Comparison

When a `push_back` triggers a reallocation, the capacity is multiplied by a **growth factor** `g`. The choice of `g` involves a fundamental tradeoff between memory waste and reallocation frequency:

| Growth Factor `g` | Reallocations for N=10⁶ | Peak unused capacity | Memory reuse possible? |
|:---|:---|:---|:---|
| 1.5× | 35 | Up to 33% | Yes (after ~4 reallocations) |
| 1.75× | 25 | Up to 43% | Partial |
| 2× | 20 | Up to 50% | No (never) |

### 10.2 Memory Waste vs Reallocation Count

**More aggressive growth (higher `g`)**:
- ✅ Fewer reallocations → less copy overhead
- ✅ More headroom → fewer future reallocations
- ❌ More wasted memory → higher peak memory usage

**Less aggressive growth (lower `g`)**:
- ✅ Less wasted memory → lower peak usage
- ✅ Previously freed blocks can be reused by the allocator (for `g < 2`)
- ❌ More reallocations → more copy overhead

The critical insight about memory reuse: with `g = 2`, each new allocation is larger than the **sum of all previous allocations**. This means the allocator can never fit the new block into the space freed by all previous blocks combined. With `g = 1.5`, after a few reallocations, the sum of previously freed blocks exceeds the next allocation size, enabling the allocator to reuse memory.

**Mathematical proof**:

For growth factor `g`, the sum of all previous allocations after `k` reallocations is:

```
S(k) = C₀ × (1 + g + g² + ... + g^(k-1)) = C₀ × (g^k - 1) / (g - 1)
```

The next allocation size is `C₀ × g^k`.

Memory reuse is possible when `S(k) ≥ C₀ × g^k`:

```
(g^k - 1) / (g - 1) ≥ g^k
g^k - 1 ≥ g^k × (g - 1)
g^k - 1 ≥ g^(k+1) - g^k
2 × g^k - g^(k+1) ≥ 1
g^k × (2 - g) ≥ 1
```

For this to hold for large `k`, we need `2 - g > 0`, i.e., **`g < 2`**. When `g = 2`, the left side is always `g^k × 0 = 0`, which never exceeds 1.

### 10.3 Why This Implementation Uses 2x Growth

Our `Vector<T>` uses `g = 2`:

```cpp
void Vector<T>::resize() {
    currentCapacity *= 2;  // Growth factor = 2
    // ...
}
```

This provides the simplest analysis for amortized O(1) `push_back` and produces the fewest number of reallocations. The tradeoff of up to 50% unused memory is acceptable for most applications, and the doubling strategy is the most widely understood growth policy in computer science education and systems programming.

---

## 11. Mathematical Growth Factor Proof

### 11.1 Reallocation Count Formula

Starting from an initial capacity `C₀` with growth factor `g`, after `k` reallocations the capacity is:

```
C(k) = C₀ × g^k
```

To store `N` elements, we need `C(k) ≥ N`:

```
C₀ × g^k ≥ N
g^k ≥ N / C₀
k ≥ log_g(N / C₀)
```

Therefore, the number of reallocations to reach capacity `N` is:

```
k = ⌈log_g(N / C₀)⌉
```

Using the change-of-base formula:

```
k = ⌈ln(N / C₀) / ln(g)⌉
```

### 11.2 Numerical Comparison for N = 1,000,000

With initial capacity `C₀ = 1`:

| Growth Factor `g` | Reallocations `k = ⌈log_g(N)⌉` | Exact Value | Peak Capacity |
|:---|:---|:---|:---|
| 1.5 | `⌈ln(10⁶)/ln(1.5)⌉` = `⌈13.8155/0.4055⌉` = `⌈34.07⌉` = **35** | 35 | 1.5³⁵ ≈ 1,223,736 |
| 1.75 | `⌈ln(10⁶)/ln(1.75)⌉` = `⌈13.8155/0.5596⌉` = `⌈24.69⌉` = **25** | 25 | 1.75²⁵ ≈ 1,266,874 |
| 2.0 | `⌈ln(10⁶)/ln(2)⌉` = `⌈13.8155/0.6931⌉` = `⌈19.93⌉` = **20** | 20 | 2²⁰ = 1,048,576 |

**Analysis**:

- Growth factor 1.5× requires **75% more reallocations** than 2×, but wastes less memory at peak.
- Growth factor 2× achieves a peak capacity of exactly 2²⁰ = 1,048,576, with 48,576 unused slots (4.9% waste at capacity, up to 50% waste just after a reallocation).
- Growth factor 2× also has the elegant property that `2²⁰ = 1,048,576 ≈ 10⁶`, making capacity calculations easy to reason about.

### 11.3 Memory Reuse Theorem

**Theorem**: For growth factor `g`, the sum of all freed allocation sizes after `k` reallocations (starting from capacity 1) is:

```
F(k) = 1 + g + g² + ... + g^(k-1) = (g^k - 1) / (g - 1)
```

The next required allocation is `g^k`.

**For `g = 1.5`**: `F(k) = (1.5^k - 1) / 0.5 = 2 × 1.5^k - 2`

At `k = 4`: `F(4) = 2 × 5.0625 - 2 = 8.125`. Next allocation: `1.5^4 = 5.0625`. Since `8.125 > 5.0625`, the freed memory **can** be reused. ✓

**For `g = 2`**: `F(k) = (2^k - 1) / 1 = 2^k - 1`

For any `k`: `F(k) = 2^k - 1 < 2^k`. The freed memory is **always** 1 byte short of fitting the next allocation. Memory reuse is never possible. ✗

---

## 12. Amortized Analysis of push_back

The worst-case cost of a single `push_back` is O(N) — when a reallocation occurs, all N elements must be moved. However, reallocations happen exponentially less frequently as the vector grows. The **amortized** cost — the average cost per operation over a sequence of operations — is O(1).

**Proof — Geometric Series (Aggregate) Method**

**Setup**: Start with capacity 1, growth factor 2. Perform N `push_back` operations.

**Observation**: A reallocation occurs when `size = capacity`, at sizes `1, 2, 4, 8, ..., 2^(⌊log₂N⌋)`. At reallocation `k`, exactly `2^k` elements are copied.

**Total copy cost** (across all reallocations):

```
C_total = 1 + 2 + 4 + 8 + ... + 2^(⌊log₂N⌋)
```

This is a geometric series:

```
C_total = Σ_{k=0}^{⌊log₂N⌋} 2^k = 2^(⌊log₂N⌋ + 1) - 1 ≤ 2N - 1 < 2N
```

**Total insertion cost** (placement new for each element): N.

**Total cost**: `C_total + N < 2N + N = 3N`.

**Amortized cost per push_back**:

```
T_amortized = Total Cost / N < 3N / N = 3 = O(1)    ∎
```

This proves that even though individual `push_back` calls can cost O(N) in the worst case, the average cost per call over any sequence of N calls is bounded by a constant. The geometric growth strategy ensures that the expensive reallocations are spaced far enough apart that their total cost is absorbed by the many cheap (O(1)) insertions between them.

---

## 13. Reallocation Algorithm

### 13.1 Step-by-Step Process

The `resize()` method in our `Vector<T>` performs the following steps:

```
Step 1: Double the capacity
        currentCapacity *= 2;

Step 2: Allocate a new buffer of size currentCapacity × sizeof(T)
        T* newData = static_cast<T*>(malloc(currentCapacity * sizeof(T)));

Step 3: Transfer elements from old buffer to new buffer
        For trivially copyable types:
            std::memcpy(newData, data, currentSize * sizeof(T));
        For non-trivial types:
            for each element i:
                new(&newData[i]) T(std::move_if_noexcept(data[i]));

Step 4: Destroy old objects (non-trivial types only)
        for each element i:
            data[i].~T();

Step 5: Free old buffer
        free(data);

Step 6: Update pointer
        data = newData;
```

### 13.2 Trivial vs Non-Trivial Type Handling

Our implementation uses `if constexpr` to select the optimal transfer strategy at compile time:

**Trivially copyable types** (`int`, `double`, `float`, plain structs without custom copy semantics):

```cpp
if constexpr (std::is_trivially_copyable<T>::value) {
    if (currentSize > 0) {
        std::memcpy(newData, data, currentSize * sizeof(T));
    }
}
```

`std::memcpy` copies `currentSize × sizeof(T)` bytes in a single bulk operation. This is highly optimized by compilers and typically compiles to SIMD instructions (e.g., `rep movsb`, `vmovdqu` on x86). No per-element constructor calls are needed because the bitwise representation **is** the object — there are no internal pointers or invariants to maintain.

**Non-trivially copyable types** (`std::string`, any class with custom copy/move semantics):

```cpp
for (size_t i = 0; i < currentSize; i++) {
    new(&newData[i]) T(std::move_if_noexcept(data[i]));
}
```

`std::move_if_noexcept` selects move construction if `T`'s move constructor is `noexcept`, otherwise falls back to copy construction. This is critical for exception safety: if a move constructor throws, the old elements may already be in a moved-from state, making it impossible to recover. By requiring `noexcept`, we guarantee that either all elements are successfully moved or none are (because a `noexcept` move cannot fail).

---

## 14. Function-by-Function Deep Dive

### 14.1 push_back

**Purpose**: Appends a copy of the given value to the end of the vector.

**Internal Algorithm**:
1. Check if `currentSize >= currentCapacity` (buffer full)
2. If full, call `resize()` to double the capacity and transfer all existing elements to the new buffer
3. Copy-construct the value at position `data[currentSize]` via placement `new`
4. Increment `currentSize`

**Memory Behaviour**: No allocation if `size < capacity` — the element is constructed in an existing unused slot. If `size == capacity`, a new buffer of `2 × capacity × sizeof(T)` bytes is allocated via `malloc`, all existing elements are transferred to it, old objects are destroyed, and the old buffer is freed via `free`.

**Best Case**: **O(1)** — `size < capacity`: a single placement `new` call and one increment.

**Worst Case**: **O(N)** — `size == capacity`: reallocation requires transferring all N existing elements.

**Space Complexity**: O(1) without reallocation. O(N) during reallocation (old and new buffers coexist temporarily, consuming `3N × sizeof(T)` peak memory).

**Proof**: The amortized cost is O(1), proven via the Aggregate Method in [Section 12](#12-amortized-analysis-of-push_back). Total cost over N insertions is bounded by 3N. Therefore: `T_amortized = 3N / N = 3 = O(1)`. ∎

---

### 14.2 pop_back

**Purpose**: Removes the last element from the vector.

**Internal Algorithm**:
1. Check if the vector is empty; if so, throw `std::out_of_range`
2. If `T` is not trivially destructible, explicitly call the destructor of the last element via `data[currentSize - 1].~T()`
3. Decrement `currentSize`

**Memory Behaviour**: No deallocation occurs. The capacity remains unchanged. The slot at `data[currentSize]` (after decrement) transitions from containing a live object to being raw, uninitialized memory. The memory is still allocated and available for future `push_back` calls.

**Best Case**: **O(1)** — trivially destructible type: one comparison and one decrement.

**Worst Case**: **O(1)** — non-trivial type: one comparison, one destructor call, one decrement.

**Space Complexity**: O(1) — no allocation or deallocation.

**Proof**: The operation performs exactly 1 comparison, at most 1 destructor call, and 1 decrement — all O(1) operations. No loops, no recursion, no data-dependent branching. Total: O(1). ∎

---

### 14.3 insert

**Purpose**: Inserts a copy of the given value at a specified index, shifting all subsequent elements one position to the right.

**Internal Algorithm**:
1. Bounds check: if `index > currentSize`, throw `std::out_of_range`
2. Capacity check: if `currentSize >= currentCapacity`, call `resize()` to double the capacity
3. If `index == currentSize`: equivalent to `push_back` — construct the element at the end
4. Otherwise:
   - Move-construct a new element at position `currentSize` using the last element (extends the live region by one slot into previously raw memory)
   - Shift elements from `[index, currentSize - 1)` one position to the right using move assignment
   - Copy-assign the new value into the now-vacated slot at `index`
5. Increment `currentSize`

**Memory Behaviour**: No allocation if `size < capacity` — elements are shifted within the existing buffer. If reallocation occurs, a new buffer is allocated, all elements are transferred, and the old buffer is freed.

**Best Case**: **O(1)** — `index == currentSize` (insert at end, no elements to shift).

**Worst Case**: **O(N)** — `index == 0` (every existing element must be shifted one position right).

**Space Complexity**: O(1) without reallocation. O(N) if reallocation occurs.

**Proof**: Let `S = currentSize - index` be the number of elements shifted. Each shift is a single move assignment: O(1). Total shift cost: `S × O(1) = O(S)`. Worst case: `index = 0` → `S = N` → O(N). Best case: `index = currentSize` → `S = 0` → O(1). Average case (uniform random index): `E[S] = N/2` → O(N). ∎

---

### 14.4 remove

**Purpose**: Removes the element at a specified index, shifting all subsequent elements one position to the left.

**Internal Algorithm**:
1. Bounds check: if `index >= currentSize`, throw `std::out_of_range`
2. Shift elements from `[index + 1, currentSize)` one position to the left using move assignment — each element `data[i]` receives `std::move(data[i + 1])`
3. If `T` is not trivially destructible, explicitly call the destructor of the last element (now a moved-from duplicate)
4. Decrement `currentSize`

**Memory Behaviour**: No deallocation. The capacity remains unchanged. The last slot transitions from live to raw memory after destruction.

**Best Case**: **O(1)** — `index == currentSize - 1` (remove last element, no shifting required).

**Worst Case**: **O(N)** — `index == 0` (shift all `N - 1` remaining elements one position left).

**Space Complexity**: O(1) — no allocation or deallocation.

**Proof**: Let `S = currentSize - 1 - index` be the number of elements shifted. Each shift: O(1). Total: O(S). Worst case: `index = 0` → `S = N - 1` → O(N). Best case: `index = currentSize - 1` → `S = 0` → O(1). ∎

---

### 14.5 clear

**Purpose**: Destroys all elements in the vector but retains the allocated buffer for future reuse.

**Internal Algorithm**:
1. If `T` is not trivially destructible, iterate from index 0 to `currentSize - 1` and call each element's destructor via `data[i].~T()`
2. Set `currentSize = 0`

**Memory Behaviour**: The heap buffer is **not freed**. `currentCapacity` remains unchanged. All slots transition from live objects to raw memory. Future `push_back` calls will reuse the existing buffer without triggering a reallocation.

**Best Case**: **O(1)** — trivially destructible type: the destructor loop is eliminated at compile time by `if constexpr`, leaving only `currentSize = 0`.

**Worst Case**: **O(N)** — non-trivially destructible type: N individual destructor calls.

**Space Complexity**: O(1) — no allocation or deallocation.

**Proof**: For non-trivial `T`, the loop executes exactly `N` iterations, each calling one destructor (O(1)). Total: N × O(1) = O(N). For trivial `T`, the loop is compiled away: O(1). ∎

---

### 14.6 reserve

**Purpose**: Ensures the capacity is at least `newCapacity`. If the current capacity is already sufficient, this is a no-op.

**Internal Algorithm**:
1. If `newCapacity <= currentCapacity`, return immediately (no work needed)
2. Allocate a new buffer of `newCapacity` slots via `malloc`
3. Transfer all `currentSize` elements to the new buffer (using `memcpy` for trivial types or placement `new` with `std::move_if_noexcept` for non-trivial types)
4. Destroy old elements (for non-trivial types) and free the old buffer
5. Update `data` to point to the new buffer and set `currentCapacity = newCapacity`

**Memory Behaviour**: When the capacity must increase, a new buffer is allocated at the requested size. The old buffer is freed after all elements are transferred. During transfer, both buffers exist simultaneously.

**Best Case**: **O(1)** — `newCapacity <= currentCapacity` (no-op, immediate return).

**Worst Case**: **O(N)** — must transfer all N existing elements to the new buffer.

**Space Complexity**: O(1) for no-op. O(N) during transfer (old + new buffers coexist).

**Proof**: If reallocation occurs, the transfer loop runs `N` iterations, each performing one move-construction or `memcpy` of `sizeof(T)` bytes — O(1) per element. Total: O(N). If no reallocation, O(1). ∎

---

### 14.7 shrink_to_fit

**Purpose**: Reduces the capacity to match the current size, freeing unused memory.

**Internal Algorithm**:
1. If `currentCapacity > currentSize`:
   - Calculate `newCapacity = max(currentSize, 1)` (maintain invariant that capacity ≥ 1)
   - If `currentCapacity <= newCapacity`, return (already minimal)
   - Allocate a new buffer of `newCapacity` slots
   - Transfer all `currentSize` elements to the smaller buffer
   - Destroy old elements and free the old buffer
   - Update `data` and `currentCapacity`

**Memory Behaviour**: Allocates a smaller buffer, transfers elements, and frees the larger old buffer. Net effect: total heap usage decreases by `(oldCapacity - newCapacity) × sizeof(T)` bytes.

**Best Case**: **O(1)** — `capacity == size` (no-op).

**Worst Case**: **O(N)** — must transfer all N elements to a smaller buffer.

**Space Complexity**: O(N) during transfer (old + new buffers coexist).

**Proof**: Identical to `reserve` — transfer loop is O(N), no-op path is O(1). ∎

---

### 14.8 operator[]

**Purpose**: Returns a reference to the element at the given index. **No bounds checking** is performed.

**Internal Algorithm**:
1. Compute the memory address `data + index` using pointer arithmetic
2. Dereference and return the reference

**Memory Behaviour**: No allocation, no deallocation, no modification of any internal state. Pure read (or write through the returned reference).

**Best Case**: **O(1)**.

**Worst Case**: **O(1)**.

**Space Complexity**: O(1).

**Proof**: The operation computes `Address = (uintptr_t)data + index × sizeof(T)` — one multiplication and one addition, both constant-time CPU operations — then dereferences the result. No loops, no branching. O(1) regardless of vector size. ∎

---

### 14.9 at

**Purpose**: Returns a reference to the element at the given index **with bounds checking**. Throws `std::out_of_range` if the index is invalid.

**Internal Algorithm**:
1. Compare `index` against `currentSize`
2. If `index >= currentSize`, throw `std::out_of_range`
3. Otherwise, compute the memory address and return the reference (same as `operator[]`)

**Memory Behaviour**: No allocation, no deallocation, no state modification.

**Best Case**: **O(1)**.

**Worst Case**: **O(1)**.

**Space Complexity**: O(1).

**Proof**: One comparison + one pointer dereference. Both O(1). The only difference from `operator[]` is the bounds check, which adds one comparison instruction — a constant-time operation. ∎

---

### 14.10 front

**Purpose**: Returns a reference to the first element in the vector. Throws `std::out_of_range` if the vector is empty.

**Internal Algorithm**:
1. Check if `currentSize == 0`; if so, throw
2. Return `data[0]`

**Memory Behaviour**: No allocation, no deallocation.

**Best Case**: **O(1)**.

**Worst Case**: **O(1)**.

**Space Complexity**: O(1).

**Proof**: One comparison + one dereference of `data[0]`. Both O(1). ∎

---

### 14.11 back

**Purpose**: Returns a reference to the last element in the vector. Throws `std::out_of_range` if the vector is empty.

**Internal Algorithm**:
1. Check if `currentSize == 0`; if so, throw
2. Return `data[currentSize - 1]`

**Memory Behaviour**: No allocation, no deallocation.

**Best Case**: **O(1)**.

**Worst Case**: **O(1)**.

**Space Complexity**: O(1).

**Proof**: One comparison + one subtraction + one dereference. All O(1). ∎

---

### 14.12 find

**Purpose**: Returns the index of the first occurrence of the given value, or `currentSize` if the value is not found. The sentinel value `currentSize` indicates "not found," analogous to returning a past-the-end position.

**Internal Algorithm**:
1. Iterate from index 0 to `currentSize - 1`
2. At each index, compare `data[i]` with the target value using `operator==`
3. If a match is found, return the index immediately
4. If the loop completes without a match, return `currentSize`

**Memory Behaviour**: No allocation, no deallocation. Pure read-only traversal.

**Best Case**: **O(1)** — element found at index 0.

**Worst Case**: **O(N)** — element not found (full scan of all N elements) or found at the last index.

**Space Complexity**: O(1).

**Proof**: The loop runs at most `N` iterations. Each iteration performs one comparison (O(1) for types with O(1) `operator==`). Total: at most N × O(1) = O(N). ∎

---

### 14.13 contains

**Purpose**: Returns `true` if the given value exists anywhere in the vector, `false` otherwise.

**Internal Algorithm**:
1. Call `find(value)`
2. If the returned index is not equal to `currentSize`, the value was found — return `true`
3. Otherwise, return `false`

**Memory Behaviour**: No allocation, no deallocation.

**Best Case**: **O(1)** — element found at index 0 (delegated to `find`).

**Worst Case**: **O(N)** — element not found (delegated to `find`).

**Space Complexity**: O(1).

**Proof**: `contains` delegates entirely to `find`. The complexity is therefore identical to `find`: O(N) worst case, O(1) best case. ∎

---

### 14.14 size / capacity / empty

**Purpose**:
- `size()`: Returns the number of live elements (`currentSize`)
- `capacity()`: Returns the total buffer capacity (`currentCapacity`)
- `empty()`: Returns `true` if `currentSize == 0`

**Internal Algorithm**: Each function directly returns a stored member variable (or a simple comparison of one).

**Memory Behaviour**: No allocation, no deallocation, no state modification.

**Best Case**: **O(1)**.

**Worst Case**: **O(1)**.

**Space Complexity**: O(1).

**Proof**: Each function performs a single return of a stored value or a single comparison. No computation beyond a register read. O(1). ∎

---

### 14.15 Copy Constructor

**Purpose**: Creates a new `Vector<T>` that is an independent, deep copy of an existing vector.

**Internal Algorithm**:
1. Set `currentSize` and `currentCapacity` from the source
2. Allocate a new buffer via `malloc`
3. For trivially copyable types: bulk-copy with `std::memcpy`
4. For non-trivial types: iterate and copy-construct each element via placement `new`, tracking `constructedCount` for exception rollback

**Memory Behaviour**: Allocates a completely new buffer on the heap. After construction, the new vector and the source vector share no memory — they are fully independent.

**Best Case**: **O(N)** — must copy all N elements regardless.

**Worst Case**: **O(N)** — same.

**Space Complexity**: O(N) — allocates a new buffer of N slots.

**Proof**: The copy loop (or `memcpy`) processes exactly N elements. Each element copy is O(1). Total: N × O(1) = O(N). ∎

---

### 14.16 Copy Assignment Operator

**Purpose**: Replaces the contents of the current vector with a deep copy of another vector.

**Internal Algorithm**:
1. Self-assignment check: if `this == &other`, return immediately
2. If the existing buffer is large enough (`currentCapacity >= other.currentSize`):
   - Destroy all existing elements
   - Copy-construct new elements from the source into the existing buffer
3. If the existing buffer is too small:
   - Allocate a new buffer, copy elements into it
   - Only then destroy old elements and free the old buffer
   - Update `data`, `currentSize`, `currentCapacity`

**Memory Behaviour**: May or may not allocate new memory depending on whether the existing buffer can accommodate the source's elements. Old elements are always destroyed.

**Best Case**: **O(1)** — self-assignment (`this == &other`): immediate return.

**Worst Case**: **O(N)** — destroy old elements + copy N new elements (+ allocation if buffer is too small).

**Space Complexity**: O(1) if buffer is reused. O(N) if a new buffer must be allocated.

**Proof**: Self-assignment: O(1). Otherwise: destroying M old elements is O(M), copying N new elements is O(N). Total: O(M + N) = O(N) (where N is the dominant term since M ≤ old size). ∎

---

### 14.17 Destructor

**Purpose**: Destroys all live elements and frees the heap buffer, ensuring no memory leak.

**Internal Algorithm**:
1. If `data != nullptr`:
   - If `T` is not trivially destructible, iterate and call each element's destructor
   - Call `free(data)` to return the buffer to the allocator
   - Set `data = nullptr`, `currentSize = 0`, `currentCapacity = 0`

**Memory Behaviour**: All heap memory owned by the vector is released. After the destructor completes, the vector holds no resources.

**Best Case**: **O(1)** — trivially destructible type: only `free(data)` is called.

**Worst Case**: **O(N)** — non-trivially destructible type: N destructor calls + `free(data)`.

**Space Complexity**: O(1) — no allocation. Memory is only freed.

**Proof**: For trivial `T`: the destructor loop is eliminated by `if constexpr`. Only `free` is called: O(1). For non-trivial `T`: the loop calls `N` destructors, each O(1). Total: O(N). ∎

---

## 15. Complexity Proofs

This section provides consolidated formal proofs for the time complexity of each operation.

### push_back — Amortized O(1)

**Full proof provided in [Section 12](#12-amortized-analysis-of-push_back)** via the Aggregate (Geometric Series) Method. Total cost over N insertions is bounded by 3N. Amortized: 3N / N = O(1). ∎

### pop_back — O(1)

**Proof**: `pop_back` performs 1 comparison (`currentSize == 0`), at most 1 destructor call, and 1 decrement. All O(1). No loops, no recursion. Total: O(1). ∎

### insert(index, value) — O(N)

**Proof**: Elements shifted = `currentSize - index`. Each shift is a move assignment: O(1). Worst case: `index = 0` → `N` shifts → O(N). Best case: `index = currentSize` → 0 shifts → O(1). Average (uniform random index): `E[shifts] = N/2` → O(N). ∎

### remove(index) — O(N)

**Proof**: Elements shifted = `currentSize - 1 - index`. Each shift: O(1). Worst case: `index = 0` → `N - 1` shifts → O(N). Best case: `index = currentSize - 1` → 0 shifts → O(1). ∎

### find(value) — O(N)

**Proof**: Linear scan performs at most `N` comparisons. Each comparison is O(1). Worst case: element not found → `N` comparisons → O(N). Best case: element at index 0 → 1 comparison → O(1). ∎

### operator[](index) — O(1)

**Proof**: Computes `data + index` (one addition) and dereferences (one load). Both constant-time. O(1). ∎

### at(index) — O(1)

**Proof**: One comparison + one pointer dereference. O(1). ∎

### front / back — O(1)

**Proof**: One comparison + one dereference of `data[0]` or `data[currentSize - 1]`. O(1). ∎

### clear — O(N) (non-trivial) / O(1) (trivial)

**Proof**: For non-trivial `T`: loop calls `N` destructors (each O(1)). Total: O(N). For trivial `T`: `if constexpr` eliminates the loop. Only `currentSize = 0` executes. O(1). ∎

### reserve / shrink_to_fit — O(N)

**Proof**: If reallocation occurs, all `N` elements must be transferred. Each transfer: O(1). Total: O(N). If no reallocation needed: O(1). ∎

### size / capacity / empty — O(1)

**Proof**: Each returns a single stored value. No computation. O(1). ∎

### Copy Constructor — O(N)

**Proof**: Must copy all N elements. Each copy: O(1). Total: O(N). ∎

### Copy Assignment — O(N)

**Proof**: Self-assignment: O(1). Otherwise: destroy old + copy new = O(M + N) = O(N). ∎

### Destructor — O(N) (non-trivial) / O(1) (trivial)

**Proof**: For non-trivial `T`: N destructor calls + `free`. O(N). For trivial `T`: only `free`. O(1). ∎