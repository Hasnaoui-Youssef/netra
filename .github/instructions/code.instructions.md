# C++ Semantics and Coding Contract

This document defines the **semantic rules and assumptions** for all C++ code in this project.
It is not a style guide. It is a correctness and intent contract.

Failure to follow these rules is considered a bug, not a preference mismatch.

---

## 1. Language Level and Assumptions

- The project targets **C++23**.
- Code must compile cleanly with:
  - GCC (latest stable known)
  - Clang (latest stable known)
- Undefined behavior is unacceptable, even if “it works”.

If behavior depends on:
- compiler quirks
- evaluation order accidents
- unspecified behavior

it is wrong.

---

## 2. Ownership and Lifetime

### General Rules
- Ownership must always be explicit.
- Lifetimes must be obvious from local reasoning.
- Resource management must be deterministic.

### Allowed
- Stack allocation (preferred)
- `std::unique_ptr` for exclusive ownership
- References for non-owning, non-null access
- `std::span` for non-owning views

### Restricted
- `std::shared_ptr`
  Allowed only when:
  - ownership is truly shared
  - lifetime cannot be statically reasoned about

### Forbidden
- Raw owning pointers
- `new` / `delete` in application code
- Manual lifetime tracking via comments

If ownership is unclear, the design is wrong.

---

## 3. Nullability

- References are assumed **non-null**.
- Pointers are assumed **nullable**.
- Nullable pointers must be explicitly checked.

Do not encode nullability through comments.

Prefer:
- `std::optional<T>` for optional values
- `T&` when null is not valid

---

## 4. Value vs Identity

- Prefer **value semantics** by default.
- Use identity semantics only when required.

### Value Types
- Are cheap to copy or move
- Have clear invariants
- Are trivially destructible when possible

### Identity Types
- Have stable addresses
- Are not freely copyable
- Must document why identity matters

If a type is non-copyable, this must be intentional.

---

## 5. Const Correctness

- `const` means **logical immutability**, not “I didn’t feel like changing it”.
- Public APIs must be const-correct.
- Mutating through `const_cast` is forbidden.

If a function does not modify observable state, it must be `const`.

---

## 6. Error Handling

### Preferred
- Return values for expected errors
- `std::optional` or `std::expected` for recoverable failures

### Exceptions
- Allowed for:
  - Programming errors
  - Invalid states
  - Invariant violations
- Must not be used for normal control flow.

### Forbidden
- Silent failure
- Error codes without documentation
- Catch-all handlers without rethrow or handling

If an error can occur, it must be observable.

---

## 7. Undefined and Unspecified Behavior

The following are forbidden:
- Reliance on evaluation order
- Out-of-bounds access
- Type punning via `reinterpret_cast`
- Violating strict aliasing rules
- Accessing moved-from objects (except where explicitly allowed)

If correctness depends on UB not triggering, the code is broken.

---

## 8. Casting Rules

### Allowed
- `static_cast` for safe, well-defined conversions
- `const_cast` only to add constness

### Restricted
- `dynamic_cast`
  Allowed only across polymorphic boundaries with justification

### Forbidden
- `reinterpret_cast` in application code
- C-style casts

If a cast is surprising, it is probably wrong.

---

## 9. Memory and Allocation

- Allocation patterns must be intentional.
- Avoid heap allocation in:
  - Hot paths
  - Per-frame logic
  - Inner simulation loops

Custom allocators are acceptable when justified.

Do not hide allocations behind abstractions.

---

## 10. Object Construction and Destruction

- Constructors establish invariants.
- Destructors must not throw.
- Objects must not be usable in partially initialized states.

If construction can fail, this must be explicit.

---

## 11. Copying, Moving, and Triviality

- Prefer move semantics over copying when appropriate.
- If a type is expensive to copy, it must:
  - be non-copyable, or
  - clearly document the cost

Trivially destructible and trivially movable types are preferred.

---

## 12. APIs and Interfaces

- Public APIs must:
  - Minimize assumptions
  - Expose invariants
  - Avoid leaking implementation details

Do not return owning pointers.
Do not return references to temporaries.
Do not expose internal containers.

---

## 13. Concurrency and Threading

- Single-threaded determinism is the default assumption.
- Any concurrency must be:
  - Explicit
  - Isolated
  - Documented

Data races are forbidden.
Hidden background threads are forbidden.

---

## 14. Headers and Compilation

- Headers must be self-contained.
- Include what you use.
- Avoid transitive include reliance.

Forward declarations are preferred when possible.

---

## 15. What NOT To Do (C++ Specific)

- Do NOT use C-style memory management.
- Do NOT write clever template metaprogramming without justification.
- Do NOT optimize prematurely in non-critical paths.
- Do NOT hide complexity behind “helper” abstractions.
- Do NOT write code that requires a debugger to understand.

If the code is hard to reason about, it is wrong.

---

## 16. Final Rule

C++ is a sharp tool.

If a feature exists primarily to show off language knowledge rather than to solve a concrete problem, it does not belong in this codebase.

