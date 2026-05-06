# Chapter 03: Templates — Compile-Time Power

Templates are C++'s mechanism for writing code that works with any type — generating specialized machine code for each type at compile time, with zero runtime overhead. This chapter covers the full template toolkit: function and class templates, variadic templates and fold expressions, SFINAE and type traits, C++20 concepts, policy-based design, and expression templates.

**Prerequisites:** Chapter 01-memory (allocators as policy), Chapter 02-oop (CRTP uses templates).

**Time estimate:** Core = 90 minutes · Deep Dive = 4 hours · Interview = 45 minutes

**Reading paths:**
- "I need to write a constrained template" → `core.md` (Concepts section) + `examples/02_sfinae_traits.cpp`
- "I'm debugging a TMP error" → `deep-dive.md` (Two-Phase Name Lookup, SFINAE internals)
- "I need zero-copy arithmetic" → `deep-dive.md` (Expression Templates) + `examples/04_expression_templates.cpp`

**Atlas Lab:** `projects/02-foundation/include/foundation/templates/` — TypeList, SFINAE detection, policy Sorter, expression templates. Run: `ctest --preset debug -R test_templates` from `projects/02-foundation/`.
