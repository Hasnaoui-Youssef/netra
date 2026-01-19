## Things You Must NEVER Generate

This section defines explicitly forbidden patterns.
Any generated code containing these is invalid.

---

### 1. Fake or Meaningless Tests

Copilot must never generate:
- Construction-only tests
- Tests that only check non-null pointers
- Tests that rely on logging instead of assertions
- Tests that cannot fail meaningfully

Forbidden examples:

    TEST(Foo, Exists) {
        Foo f;
    }

    ASSERT_NE(ptr, nullptr);
    std::println("test passed");

If a test can pass while the implementation is broken, it must not exist.

---

### 2. Raw Ownership and Manual Memory Management

Copilot must never generate:
- new / delete
- Owning raw pointers
- Manual lifetime tracking via comments
- malloc / free

Exceptions are limited to explicitly documented low-level subsystems.

---

### 3. Header-Only Implementations by Convenience

Copilot must never:
- Place non-trivial implementations in headers for convenience
- Violate header/source symmetry
- Introduce logic in headers without justification

Templates are the only exception.

---

### 4. Blind Container Usage

Copilot must never:
- Default to std::map or std::unordered_map without justification
- Use associative containers where flat storage would suffice
- Introduce containers without reasoning about access patterns

If a container choice is not explained, it is wrong.

---

### 5. Simulation Semantics Violations

Copilot must never:
- Ignore cycle-level semantics
- Collapse combinational and sequential logic
- Mutate simulation state implicitly
- Introduce hidden ordering dependencies

Dataflow and clocking are first-class concepts.

---

### 6. Hidden Global State

Copilot must never:
- Introduce global mutable variables
- Hide state in anonymous namespaces without justification
- Rely on static initialization order

All state must be explicit and owned.

---

### 7. Over-Abstraction and Design Pattern Abuse

Copilot must never:
- Introduce factories, visitors, or observers "just in case"
- Add abstraction layers without measurable benefit
- Use inheritance where composition suffices

Abstractions must pay rent.

---

### 8. Clever C++ Tricks

Copilot must never:
- Use template metaprogramming for expressiveness alone
- Introduce SFINAE-heavy code without necessity
- Use reinterpret_cast
- Use C-style casts

If the code requires language-lawyer reasoning, it does not belong here.

---

### 9. Concurrency Without Consent

Copilot must never:
- Introduce threads implicitly
- Use atomics without documented memory ordering
- Add background tasks or async logic by default

Determinism is not optional.

---

### 10. UI Logic Leaks

Copilot must never:
- Put simulation logic inside ImGui callbacks
- Bind engine state directly to UI widgets
- Encode behavior in rendering code

UI reflects state. It does not control it.

---

### 11. Magic and Silent Behavior

Copilot must never:
- Introduce hidden side effects
- Perform implicit allocations in hot paths
- Rely on undocumented behavior
- Add "helper" utilities that obscure control flow

If behavior is not obvious, it is wrong.

---

### 12. Multiple Tasks From a Single Prompt

If a user prompt implies multiple independent tasks, Copilot must never:
- Start implementing any of them
- Combine tasks into a single solution
- Assume priority or intent

Copilot must:
1. List the inferred tasks
2. Ask the user to select one
3. Stop execution until a single task is chosen

Producing code for multiple tasks from one prompt is a hard violation.

---

### 13. Placeholder or Performative Code

Copilot must never:
- Generate TODO-heavy scaffolding
- Add placeholder logic "for now"
- Produce code that exists only to compile

All code must either implement real behavior or be explicitly documented as incomplete.

---

### Final Enforcement Rule

If Copilot generates code violating any rule in this section, the output must be discarded.

These are constraints, not preferences.

