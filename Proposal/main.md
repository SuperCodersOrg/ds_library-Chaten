# Data Structure Library - Architectural Design Proposal

**Project:** Project 01 - Collections Library & Redis Lite

## 1. Project Overview & Objective

### Objective
The primary objective of this project is to build a robust, high-performance C++ collections library from scratch, culminating in a practical application (Redis Lite). By avoiding standard library containers like `std::vector` or `std::unordered_map`, this project demonstrates a deep understanding of low-level memory management, pointer arithmetic, algorithmic complexity, object lifecycles, and modern C++ optimizations.

### Behind the Scenes: Flow of the Build
Building this library was a journey of solving progressively harder problems. The development flow for the final product went through several architectural phases:

1. **Phase 1: Fixed limits.** Started by experimenting with basic arrays and simple structs. 
   * *Problem:* Hardcoded size limits caused buffer overflows or wasted massive amounts of memory.
2. **Phase 2: Dynamic Allocation.** Moved to dynamically allocated memory using `malloc` and `free`.
   * *Problem:* Reallocating memory every single time a user added an element was too slow.
3. **Phase 3: Smart Growth Strategies.** Implemented capacity doubling (geometric expansion) to balance memory usage and CPU speed.
   * *Problem:* Copying complex objects during resizing was causing memory leaks if exceptions were thrown.
4. **Phase 4: Modern C++ Polish.** Integrated advanced concepts like placement `new`, explicit destructors `~T()`, `std::memcpy` for fast primitive copies, and move semantics (`std::move`) to make the structures as fast and safe as production-grade C++ standard libraries.

## 2. Table of Contents
Please refer to the specific documentation files for deep-dive architectural details, mathematical proofs, and memory diagrams for each component:

1. [Dynamic Array (Vector) Architecture](./DynamicArray.md)
2. [Doubly Linked List Architecture](./List.md)
3. [Hash Map Architecture](./HashMap.md)
4. [Redis Lite Application](./RedisLite.md)

## 3. Memory Management: The Rule of Three (and Five)
Because all of these data structures manage raw heap memory using `malloc`/`free` or placement `new`, they strictly adhere to C++ resource management rules to prevent memory leaks and dangling pointers.

* **Destructors (`~T() noexcept`):** Every class explicitly cleans up its memory. The Vector destroys elements and frees the array; the List deletes every node; the HashMap clears all buckets and collision chains.
* **Copy Constructor (Deep Copies):** Allocates brand new heap memory and manually copies data over, ensuring each object owns its own isolated data (preventing Double Free Corruption).
* **Copy Assignment Operator (`operator=`):** Safely checks for self-assignment, allocates new memory *first* (for strong exception safety), destroys old memory, and points to the new block.
* **Move Semantics:** Internally utilizes `std::move_if_noexcept` to steal pointers instead of deep-copying massive objects during array resizing or node shifting, guaranteeing standard-library level performance.
