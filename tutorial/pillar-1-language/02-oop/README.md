# Chapter 02: OOP — When Inheritance Actually Helps

Object-oriented programming in C++ is not the same as OOP in Java or Python. C++ OOP is about choosing the right level of abstraction for a problem — and understanding the cost of each choice. This chapter covers the Rule of Five for resource management, move semantics for performance, virtual dispatch for runtime polymorphism, and CRTP for zero-overhead compile-time polymorphism.

**Prerequisites:** Chapter 01-memory (ownership and RAII underpin all OOP design decisions).

**Time estimate:** Core = 90 minutes · Deep Dive = 2.5 hours · Interview = 30 minutes

**Reading paths:**
- "I keep hitting copy/move bugs" → `core.md` (Rule of Five, Move Semantics)
- "I need to choose between virtual and CRTP" → `core.md` (Virtual vs CRTP decision tree)
- "I'm designing a library with ABI stability" → `deep-dive.md` (PIMPL, vtable layout)
- "Interview tomorrow" → `interview.md` (8 Q&A pairs)

**Atlas Lab:** `projects/02-foundation/include/foundation/oop/` — Rule of 0/3/5 showcase, CRTP mixins, NVI, and diamond problem solutions. Run: `ctest --preset debug -R test_oop` from `projects/02-foundation/`.
