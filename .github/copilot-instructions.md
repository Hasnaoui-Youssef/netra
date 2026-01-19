# Copilot Coding Agent Instructions

## 1. Project Scope and Intent

This project is a **GUI-based RTL design and simulation system**.

- The current and only target is **simulation**.
- There is **no synthesis, hardware generation, FPGA, or ASIC flow** in scope.
- The system focuses on **cycle-accurate, dataflow-oriented evaluation** of RTL-like systems.
- The project prioritizes:
  - Determinism
  - Predictability
  - Inspectability
  - Performance transparency

Any feature, abstraction, or design that obscures dataflow or cycle-level behavior is considered incorrect.

---

## 2. Language, Standard, and Build System

- **Language:** C++23
- **Build System:** CMake (Modern)

### Constraints
- No compiler-specific extensions unless explicitly gated.
- Prefer standard C++23 facilities over external utilities.
- No global CMake flags.
- No legacy CMake patterns.
- No magical auto-discovery logic.

---

## 3. Graphics and UI Stack

### Rendering
- OpenGL is the rendering backend.
- Avoid deprecated functionality.
- State changes must be explicit and minimal.

### Windowing and Input
- GLFW is used strictly for:
  - Window creation
  - Input
  - Context management

### GUI
- ImGui is used for **all** GUI functionality.

### UI Layout Rules
- The graphical interface follows an **IDE-like layout**:
  - A **central canvas** is the primary workspace.
  - Supporting panels include (but are not limited to):
    - Gate/component palette
    - Property inspector
    - Simulation controls
    - Debug views
- The canvas is where:
  - Dataflow graphs are created
  - Connections are edited
  - Simulation state is visualized

UI code must never contain simulation logic.

---

## 4. External Dependencies

Approved dependencies:
- Boost
- GLFW
- ImGui
- GLM
- CTest

Rules:
- Do not introduce new dependencies without explicit justification, you should suggest and discuss with the user before adding them.
- Do not leak dependency-specific types into engine-facing APIs.
- Prefer standard library equivalents whenever possible.

---

## 5. Engine Architecture

### ECS (Entity-Component-System)

- **Entities**
  - Opaque identifiers only
  - No logic
  - No ownership

- **Components**
  - Plain data
  - No behavior
  - Designed for cache efficiency

- **Systems**
  - Own all behavior
  - Explicit execution order
  - Operate on component sets

Inheritance-based component hierarchies are not allowed.

---

## 6. Simulation Model

- Simulation is:
  - Cycle-based
  - Deterministic
  - Dataflow-oriented
- State updates must be:
  - Explicit
  - Ordered
  - Observable

There must be a clear separation between:
- Combinational evaluation
- Sequential (clocked) state updates

Pretending dataflow or cycle-level semantics do not exist is a design error.

---

## 7. Performance Philosophy

- Optimize for:
  - Cache locality
  - Predictable memory access
  - Compiler optimizations

### Containers
- Do not default to `std::map` or `std::unordered_map`.
- Evaluate access patterns before choosing containers.
- Flat and contiguous storage is preferred.
- Custom storage/container solutions are welcome but must be discussed first.

### Memory
- Avoid heap allocation in:
  - Per-frame code
  - Inner simulation loops
- Custom allocators are acceptable when justified.

Abstractions must justify their runtime cost.

---

## 8. Iterative Development Rule

Every non-trivial change must be evaluated in **two passes**:

1. Initial implementation.
2. A second review pass performed **without referring to the original context**, focused on:
   - Simplification
   - Performance
   - Correctness
   - Hidden assumptions
   - Potential mitigations

If no improvement is found, this must be a deliberate conclusion, not an omission.

---

## 9. Testing

- Core engine and simulation logic must be testable without the GUI.
- Use CTest.
- Tests must not depend on rendering or windowing.

Every test must answer at least one of these questions:
- What observable behavior should occur?
- What invariant must always hold?
- What must not happen?
- What breaks if this code regresses?

If it doesn’t answer one of those, delete it.

1. Ban construction-only tests
This must be a hard rule:
```c++
//Not a test
TEST(Foo, CanBeConstructed) {
    Foo a;
}
```
Construction is not behavior. It’s a side effect of linking.

If construction matters, test what construction guarantees.
```c++
//Potentially useful test
TEST(Foo, StartsInIdleState) {
    Foo a;

    EXPECT_EQ(a.state(), Foo::State::Idle);
    EXPECT_EQ(a.pending_events(), 0);
}
```
2. Test invariants, not functions
Tests should validate properties that must always hold, not just “call function, check return”.
Bad:
```c++
EXPECT_EQ(f.tick(), 0);
```
Good:
```c++
f.tick();

EXPECT_TRUE(f.is_consistent());
EXPECT_EQ(f.current_cycle(), 1);
EXPECT_FALSE(f.has_pending_combinational_updates());
```
If your code doesn’t expose invariants, that’s a design flaw.

3. Force failure paths to run
```c++
TEST(Scheduler, RejectsCombinationalLoop) {
    Graph g;
    g.connect(a, b);
    g.connect(b, a);

    EXPECT_THROW(g.validate(), CombinationalLoopError);
}
```
If you can’t write this test, you either:
- didn’t design error handling, or
- are hiding undefined behavior behind optimism

Both are bad.

4. Use state transitions, not logging

This garbage:
```c++
std::println("Success omg test passed");
```
should be banned.

A test passes **only** because assertions held.

If you need logging to feel confident, your assertions are weak.

6. Make tests hostile

A good test tries to break the code.

Examples:

- Call methods in the wrong order
- Advance time without prerequisites
- Feed invalid graphs
- Reuse moved-from objects
- Re-run simulation steps twice
```c++
TEST(Simulator, DoubleTickIsRejected) {
    Simulator sim;
    sim.tick();

    EXPECT_THROW(sim.tick(), InvalidStateTransition);
}
```
7. Prefer black-box tests for systems

For ECS and simulation:
- Don’t test internal containers.
- Don’t test implementation details.
- **Test observable behavior across cycles.**
```c++
TEST(Dataflow, RegisterUpdatesOnlyOnClockEdge) {
    Sim sim;

    sim.set_input("a", 1);
    sim.evaluate_combinational();

    EXPECT_EQ(sim.read("out"), 0);

    sim.clock();

    EXPECT_EQ(sim.read("out"), 1);
}
```
9. Add a “why would this fail?” comment
```c++
// This test fails if:
// - cycle ordering changes
// - combinational writes leak into sequential state
// - reset semantics regress
TEST(...)
```
Finally:
If your tests feel boring to write, they’re probably good.
If they feel “impressive”, verbose, or self-congratulatory, they’re theater.
---

## 10. What NOT To Do (Hard Rules)

### Architectural Violations
- Do NOT mix GUI code with simulation logic.
- Do NOT hide simulation behavior behind UI callbacks.
- Do NOT introduce implicit global state.
- Do NOT introduce inheritance-heavy designs for components.

### Simulation Violations
- Do NOT obscure dataflow semantics.
- Do NOT blur combinational and sequential logic.
- Do NOT introduce nondeterministic behavior.
- Do NOT mutate simulation state from multiple uncontrolled contexts.

### Performance Violations
- Do NOT allocate memory in hot paths without justification.
- Do NOT use virtual dispatch in inner loops.
- Do NOT introduce abstractions without measuring or reasoning about cost.

### Dependency Abuse
- Do NOT introduce new libraries “for convenience”.
- Do NOT use Boost where the standard library is sufficient.
- Do NOT leak third-party types into core engine APIs.

### UI and UX Violations
- Do NOT treat the canvas as a secondary element.
- Do NOT design layouts that obscure the dataflow graph.
- Do NOT turn the UI into a form-based configuration tool.
- Do NOT hardcode layout logic without clear structure.

### Task Discipline (Critical)
- If a user prompt implies **multiple independent tasks**:
  - You MUST NOT start implementing anything.
  - You MUST list the inferred tasks.
  - You MUST ask the user to select **one** task.
- You are strictly limited to **one coherent task at a time**.
- If necessary, you must terminate without producing code rather than working on multiple tasks from a single prompt.

Failure to follow this rule invalidates the response.

---

## 11.Header / Source Separation Rules

The project follows a **strict header/source separation model**.

### Directory Layout

The project currently contains the following top-level directories:

- `engine`
- `app`
- `common`
- `glad`
- `graphics`
- `shaders`
- `tests`

Each directory follows the same internal structure:

module/
include/
src/


---

### General Rules

- Every non-template type or function **must** have:
  - A declaration in `include/`
  - A definition in `src/`
- Header and source files must be **symmetrical**:
  - `foo.hpp` → `foo.cpp`
  - `bar.hpp` → `bar.cpp`

If a source file exists without a corresponding header, or vice versa, it is considered a structural error.

---

### Headers (`include/`)

Headers must:
- Contain **declarations only**
- Avoid implementation details
- Be self-contained
- Compile on their own
- Always have the `.hpp` extension, never the `.h`.

Forbidden in headers:
- Heavy implementation logic
- Hidden allocations
- Static mutable state
- Non-trivial function bodies (except where explicitly allowed)

Inline functions are allowed **only** when:
- They are trivial
- They are performance-critical
- They are required for templates

---

### Source Files (`src/`)

Source files must:
- Contain all non-trivial implementations
- Include their corresponding header first
- Avoid leaking private dependencies through headers

No logic should exist solely because “it was convenient to put it there”.

---

### Templates and Exceptions

Templates are the **only** allowed exception to strict separation.

- Template definitions may live in headers or `.inl` files.
- Template-heavy code must be isolated and justified.
- Do not template code that does not benefit from it.

---

### Dependency Boundaries

- Headers must not include unnecessary headers.
- Prefer forward declarations in headers.
- Includes must not cross module boundaries without justification.

If including a header pulls in half the project, the design is wrong.

---

### Tests

- Test code follows the same separation rules.
- Tests must not access private headers or internal implementation details unless explicitly intended.

---

### Final Rule

If a change breaks header/source symmetry, the change is invalid.

This is not a style preference.
It is an architectural constraint.

## 12. General Coding Rules

- Favor clarity over cleverness.
- No silent hacks.
- No unexplained magic numbers.
- No TODOs without context.
- Every non-obvious decision must be documented.

If something is slow, say it.
If something is unsafe, say it louder.

