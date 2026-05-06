# Chapter 05: Design Patterns — The C++ Edition

GoF patterns were written for Java in 1994. C++ is not Java. Many GoF patterns are unnecessary in C++ because the language already provides the mechanism. Some are better — templates and zero-cost abstractions let you do things at compile time that GoF could only describe at runtime. A few are actively dangerous if applied naively. This chapter covers all three categories, then drills into the five patterns that appear constantly in production C++ codebases.

**Prerequisites:**
- Chapter 01-memory (RAII, smart pointers — you must understand ownership before you can design with it)
- Chapter 02-oop (virtual dispatch, CRTP — required to understand why type erasure is different)
- Chapter 03-templates (type-level programming — required for policy pattern and CRTP decorator)

**Time estimate:** Core = 45 minutes · Deep Dive = 2.5 hours · Interview = 30 minutes

**Table of Contents:**

| File | What It Covers |
|------|----------------|
| `core.md` | Which GoF patterns C++ makes unnecessary, better, or dangerous. The 5 patterns you actually use. Production rules. |
| `deep-dive.md` | Type erasure internals, SBO, self-registering factory, thread-safe observer, CRTP decorator, command+undo, ECS data layout. |
| `interview.md` | 8 Q&A with traps — type erasure vs vtable, singleton harm, thread-safe observer, ECS, SBO, CRTP decorator, polymorphism trade-offs. |
| `examples/01_type_erasure.cpp` | Pedagogical `AnyCallable<void()>` — Concept+Model pattern from scratch. |
| `examples/02_self_reg_factory.cpp` | Self-registering Shape factory — Meyers singleton map, static initializer registration. |
| `examples/03_ecs_world.cpp` | Minimal ECS world — entities, components, typed storage, view queries, game loop. |

**Reading paths:**

- "I keep seeing `std::function` in code reviews — how does it work?" → `core.md` (Type Erasure) + `examples/01_type_erasure.cpp` + `deep-dive.md` (Concept+Model, SBO)
- "My factory has a 200-line if-else chain" → `core.md` (Factory) + `examples/02_self_reg_factory.cpp` + `deep-dive.md` (Self-Registering Factory)
- "I need to simulate 50,000 entities at 60 fps" → `core.md` (ECS) + `examples/03_ecs_world.cpp` + `deep-dive.md` (ECS data-oriented design)
- "The interviewer asked me about singletons" → `interview.md` Q3

**Atlas Lab:** `projects/02-foundation/include/foundation/patterns/` implements `AnyCallable<Ret(Args...)>`, `AnimalFactory` (self-registering), and an observer. Run: `ctest --preset debug -R test_patterns` from `projects/02-foundation/`.
