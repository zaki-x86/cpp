# Hardcore C++ Fundamentals (Compiler Track)

This is a **docs-first, compiler-first** C++ track that treats C++ as a language of **contracts, invariants, and failure modes**. The goal is not vibes or "getting it to work"—it’s building the mental model that lets you predict behavior, debug confidently, and write code that stays correct under optimization, refactors, and concurrency.

## Positioning

- **Hardcore**: we emphasize *what must be true*, *what can go wrong*, and *what the standard/compiler is allowed to do*.
- **Fundamentals**: object model, lifetimes, value categories, layout, ABI-ish realities, and the standard library as an algorithmic toolbox.
- **Compiler track**: you learn the language the way the compiler sees it: translation units, instantiation, UB surfaces, and observable behavior.

## Toolchain constraints (this workspace)

This workspace targets **GCC 11** with modern C++ enabled. Some C++ library features are **not available** in GCC 11’s standard library, notably:

- `std::format` (needs newer libstdc++)
- `std::expected` (C++23; needs newer toolchain)

When a concept is best illustrated with these APIs, we’ll either provide an alternative, or treat it as a *forward-looking note* while keeping the core learning path compatible with GCC 11.

## How to use this track

### Core order (read in order)

The **Core** is the backbone. Read it sequentially; later booklets assume earlier invariants.

- **Read for invariants**: what the program *must* guarantee vs what it *assumes*.
- **Read for failure modes**: UB triggers, lifetime leaks, aliasing surprises, data races.
- **Practice by modification**: after each section, change one assumption and predict the new behavior.

### Deep dives (index + when to branch)

Deep dives are for when the core model is stable and you want to go sharper in one axis (templates, perf, concurrency, stdlib internals). Don’t treat them as optional “extras”—treat them as **focused amplifiers**.

### Drills vs capstones

- **Drills**: small, repeatable exercises that enforce a single invariant (e.g. “lifetime is a graph”, “value category drives overload resolution”). You should be able to do drills quickly and explain the *why*.
- **Capstones**: larger integrations that force you to maintain correctness under multiple constraints (performance, error handling, ownership, concurrency). Capstones are where hidden assumptions surface.

## Table of contents

### Core

- [01 — Object model and lifetime](core/01-object-model-and-lifetime.md)
- TODO — Values, references, and value categories
- TODO — Initialization, implicit conversions, and narrowing
- TODO — Move semantics and ownership (Rule of Zero)
- TODO — Exceptions, error handling, and contracts
- TODO — Concurrency fundamentals (happens-before, data races)

### Deep dives (planned; created in Task 4)

- TODO — Language deep dive (value categories, overload sets, ADL)
- TODO — Memory deep dive (allocators, aliasing, object representation)
- TODO — Templates deep dive (instantiation model, concepts, diagnostics)
- TODO — Standard library deep dive (iterators, ranges, complexity contracts)
- TODO — Performance deep dive (optimization model, cache behavior, profiling)
- TODO — Concurrency deep dive (atomics, memory ordering, lock-free pitfalls)
- TODO — Toolchain deep dive (compilation model, linking, build graph)

### Legacy

Legacy chapters are the older, linear write-up. Keep them as a reference, but treat the Core + Deep Dives as the maintained path.

- [Legacy index](legacy/README.md)
