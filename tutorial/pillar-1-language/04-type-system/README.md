# Chapter 04: Type System — Making Illegal States Unrepresentable

The C++ type system is not just a mechanism for catching typos. Used deliberately, it makes incorrect programs impossible to compile. This chapter covers strong types, `std::optional`, `std::variant`, monadic error handling, and phantom types — the toolkit for encoding program invariants in the type system itself.

**Prerequisites:** Chapter 02-oop (constructors, CRTP), Chapter 03-templates (template metaprogramming).

**Time estimate:** Core = 60 minutes · Deep Dive = 2.5 hours · Interview = 30 minutes

**Reading paths:**
- "I keep mixing up int parameters" → `core.md` (Strong Typedefs) + `examples/01_strong_type.cpp`
- "I need to model mutually exclusive states" → `core.md` (variant + state machine) + `examples/02_variant_visitor.cpp`
- "I want composable error handling without exceptions" → `deep-dive.md` (monadic ops) + `examples/03_expected_chain.cpp`

**Atlas Lab:** `projects/02-foundation/include/foundation/types/` — `StrongType<T,Tag>`, `std::variant` visitor patterns, `Expected<T,E>` monad. Run: `ctest --preset debug -R test_types` from `projects/02-foundation/`.
