---
title: Hardcore C++ Fundamentals (Compiler Track)
date: 2026-05-06
owner: iris
status: draft
---

## Goal

Turn `projects/learn-cpp-beginner/` from a light intro into a **self-contained**, **hardcore** C++ fundamentals guide aimed at **building a compiler** (lexer → parser → AST → type checking → IR → optimizations), while still being readable in order.

Success looks like:

- A coherent set of “core” booklets that teach the *model* of C++ (lifetime, types, templates, memory model, standard library contracts).
- A large set of “deep dives” that cover breadth across language + standard library + best practices + pitfalls.
- Each core booklet ends with a compiler-shaped capstone; deep dives end with drills.
- No dependency on other repo docs/code to understand the material (no “go read X elsewhere” required).

Non-goals:

- Not a generic “C++ cookbook”.
- Not a thin glossary of features.
- Not primarily a build-system tutorial (toolchain coverage is included only as necessary for correctness/perf/UB).

## Constraints

- Toolchain in this repo is GCC 11 (so avoid teaching APIs that don’t exist in GCC 11’s libstdc++ as if they were available, e.g. `std::format`, `std::expected`).
- Target audience: production engineer (Python background) new to C++ in production; wants systems topics: compilers, embedded, performance, OS internals, distributed systems.
- Documents must remain maintainable: consistent templates, clear TOC, and stable naming.

## Approach (Approved)

Rewrite `projects/learn-cpp-beginner/` in-place into a **Hybrid** structure:

- **Core booklets (12–16)**: long-form narrative, tight sequencing, mental models, invariants, and capstones.
- **Deep dives (40–80)**: short, indexed references that cover breadth, each with drills for fluency.

This is “Option 1”: a cohesive self-contained book+handbook hybrid inside the existing folder.

## Information Architecture

### Directory layout (within `projects/learn-cpp-beginner/`)

- `README.md`
  - Rebranded as “Hardcore C++ Fundamentals (Compiler Track)”
  - Contains:
    - reading paths (core order; “lookup only” deep-dive index)
    - explicit toolchain notes (GCC 11 constraints)
    - how to practice (drills + capstones)

- Core booklets: `core/NN-<slug>.md`
- Deep dives: `dives/<area>/NNN-<slug>.md`

Where `<area>` groups by topic for discoverability:
- `dives/language/`
- `dives/stdlib/`
- `dives/templates/`
- `dives/memory/`
- `dives/concurrency/`
- `dives/toolchain/`
- `dives/perf/`

### Naming conventions

- Core: `core/01-object-model-and-lifetime.md`, …, `core/14-capstone-mini-compiler.md`
- Dives: `dives/language/010-value-categories-cheatsheet.md`, `dives/stdlib/120-vector-invalidation-and-complexity.md`, etc.

## Content Design

### Core booklet template (long-form)

Each core booklet includes:

1. **Why you care (compiler-shaped)**
2. **Mental model**
3. **Rules & invariants** (what the language guarantees; what is UB)
4. **Common failure modes** (what bites pros; how to debug)
5. **Idioms** (the “do this, not that” with rationale)
6. **Standard library contracts used here** (complexity, invalidation, exception safety, iterator rules)
7. **Drills (optional small)** (5–10)
8. **Capstone (few but hard)** (1–2) — directly helpful for compiler work

### Deep dive template (short-form)

Each deep dive includes:

- **One concept, deeply**
- **Rules/guarantees**
- **Pitfalls**
- **Minimal example(s)**
- **Drills (many small)** (5–15)
- **“Used in compilers as…”**: 1–3 bullets mapping to compiler work

### Exercises strategy (compiler goal)

- Deep dives: many small drills for fluency.
- Core booklets: capstones that integrate concepts into compiler components.
- Global progression: drills → capstones → mini-compiler.

## Planned Coverage (High-level TOC)

This section enumerates *what will be covered*; it is intentionally broad to satisfy “touch upon every single concept/practice/standard/stdlib” over time, without claiming to be exhaustive in one pass.

### Core booklets (proposed 14)

1. **Object model & lifetime**: storage duration, RAII, temporaries, references, copy/move, copy elision, exceptions & unwinding basics.
2. **Types & expressions**: cv/ref, pointers/arrays/functions, overload resolution (practical), implicit conversions, `constexpr` mental model.
3. **Interfaces & invariants**: classes, special members, rule of zero, value vs reference types, error handling strategy.
4. **STL as contracts**: iterators, ranges (where available), algorithms, complexity, invalidation rules, string vs string_view vs span.
5. **Ownership & allocation**: unique/shared/weak, arenas/pools conceptually, allocator model (practical), small-object optimization intuition.
6. **Error handling**: exceptions vs status types, strong/basic/no-throw guarantees, designing APIs for compilers.
7. **Templates I (mechanics)**: deduction, instantiation, specialization, `decltype`, `std::declval`, forwarding references.
8. **Templates II (design)**: constraints/concepts, detection idiom, type traits, compile-time computation patterns.
9. **Sum types & dispatch**: `variant`, visitors, pattern-matching style, type erasure, virtual vs templates trade-offs.
10. **Concurrency & memory model**: data races, happens-before, mutexes, atomics basics, memory orders overview, false sharing.
11. **Performance model**: layout/alignment, caches, branch prediction, allocations, inlining/code size, measurement basics.
12. **Toolchain reality**: compilation/linking model, ODR, ABI-ish constraints, sanitizers, UB taxonomy and how tools surface it.
13. **Systems C++ patterns**: intrusive structures, arenas, string interning, flyweight, non-owning views, “parse fast” patterns.
14. **Capstone: mini-compiler**: lexer → parser → AST → diagnostics → (optional) simple IR + pass pipeline.

### Deep dives (initial index targets)

Language:
- Value categories & reference collapsing
- Temporaries and lifetime extension rules
- Undefined behavior taxonomy (core language)
- Integer promotions, signed overflow, bit operations
- Initialization forms, narrowing, aggregate init
- `constexpr` vs `consteval`, compile-time evaluation rules
- `noexcept` and its optimization impact
- Copy elision & NRVO (what’s guaranteed vs not)
- Exceptions: stack unwinding, RAII interactions, cost model (practical)

Stdlib:
- `vector`/`string` invalidation + growth + complexity
- `unordered_*` rehashing costs and iterator invalidation
- `map`/`set` vs `unordered_*` trade-offs (compilers)
- `string_view` pitfalls and ownership boundaries
- `span` (where available) and view patterns
- Algorithms: sort/stable_sort/partition/lower_bound; iterator category requirements
- `pmr` overview (even if not used) as design idea

Templates:
- SFINAE/detection idiom (why it existed)
- Concepts and readable constraints (C++20)
- `std::type_traits` as a toolbox (common ones, how to combine)

Concurrency:
- Mutex patterns, lock ordering, deadlocks
- Atomics and memory order cheat sheet
- ABA problem and “lock-free is hard”

Perf/toolchain:
- Layout/alignment/padding; `std::byte`
- Sanitizers (ASan/UBSan/TSan) and how to interpret findings
- Compile time vs runtime trade-offs; templates and code bloat

Compiler-specific:
- Arena allocation for AST nodes
- String interning table design
- Diagnostic design (spans, source locations)
- Visitor-based AST traversal vs virtual dispatch vs pattern matching

## Rollout Plan (Editorial)

1. Update `projects/learn-cpp-beginner/README.md` to reflect new structure, TOC, and practice rules.
2. Create `core/` and `dives/` directories and seed:
   - Core booklet 01
   - 5–10 deep dives that support it
3. Iterate in cycles:
   - Add 1 core booklet
   - Add 5–12 supporting deep dives
   - Ensure TOC stays accurate

## Quality Bar

- Every booklet must contain at least:
  - a mental model
  - at least one “this is where people get it wrong” section
  - drills (deep dives) or a capstone (core)
- Avoid “term soup”. Prefer constraints, invariants, and failure modes.
- Keep examples GCC 11 compatible; explicitly label newer features as “not available on this toolchain” when relevant.

## Open Questions / Future

- Whether to introduce a standalone `projects/<NN-compiler>` project later for the capstone implementation (separate from this documentation rewrite).

