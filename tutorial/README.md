# The C++ Bible — A Complete Systems Engineering Encyclopedia

This is not a gentle introduction. It is an opinionated, comprehensive reference for engineers who want to understand C++ the way senior systems programmers actually think about it — from the machine up, not from the syntax down. Every chapter covers theory, tooling, real examples you can compile with `g++ -std=c++20`, and interview preparation written from genuine production experience.

## How to Use This Bible

Start at 00-orientation regardless of your experience level. Even greybeards find it useful to revisit the mental models. Then follow one of the reading paths below or read straight through. Each chapter has three layers: **Core** for the essential mental model (30–60 min), **Deep Dive** for reference-grade detail (1–3 hours), and **Interview** for targeted Q&A you can practice the night before a loop. You do not have to read all three layers — pick the depth you need and move on. The goal is that you leave every chapter knowing the concept well enough to teach it.

---

## Reading Paths

### "Interview in 3 days"
Chapters: 00 → 01 → 02 → 03 → 04 → 06 → 07 → 08 → interview-master/

Focus exclusively on `core.md` and `interview.md` in each chapter. Skip deep-dive sections entirely. Do the interview-master Q&A bank last as a timed drill.

### "Systems Programming deep dive"
Chapters: 00 → 01 → 06 → 08 → 09 → 10 → 11 → 12 → 13 → 14 → 15

Read all three layers in pillar-1 and pillar-3. For pillar-2 read core.md and deep-dive.md. This path makes you dangerous in performance-critical systems work.

### "Robotics track"
Chapters: 00 → 01 → 02 → 06 → 08 → 12 → 13 → 18 → 19

Complement with Atlas's `projects/03-ros2` lab. Read all three layers in 18 and 19. This path prepares you for robotics software engineering interviews at Boston Dynamics, Waymo, or similar.

### "Graphics track"
Chapters: 00 → 01 → 06 → 08 → 10 → 12 → 21 → 22 → 23

Read deep-dive.md in chapter 10 (profiling) and chapter 21 (OpenGL). The graphics track requires understanding the memory model and threading model before you touch any GPU API.

---

## Chapter Index

| # | Title | One-sentence description |
|---|-------|--------------------------|
| **Pillar 1 — Language** | | |
| 00 | Orientation: How C++ Thinks | The C++ contract, the compilation pipeline, and the mental models that make everything else click. |
| 01 | Memory: The Foundation of Everything | Stack, heap, RAII, smart pointers, allocators, and why memory is the lens through which C++ must be understood. |
| 02 | OOP: When Inheritance Actually Helps | Vtables, virtual dispatch, the cost of polymorphism, and when to use composition instead. |
| 03 | Templates: Compile-Time Power | Function and class templates, SFINAE, concepts (C++20), template metaprogramming, and type erasure. |
| 04 | Type System: Making Illegal States Unrepresentable | `const`, `constexpr`, `consteval`, strong typedefs, `std::variant`, `std::optional`, and type-driven design. |
| 05 | Design Patterns: The C++ Edition | GoF patterns in modern C++, policy-based design, CRTP, mixins, and when patterns hurt more than they help. |
| 06 | Concurrency: Writing Correct Multi-Threaded Code | The memory model, `std::thread`, `std::jthread`, mutexes, atomics, lock-free data structures, coroutines. |
| 07 | Modern C++: 11 Through 23 Feature by Feature | Move semantics, lambdas, ranges, modules, `std::span`, `std::bit_cast`, and what each standard actually changed. |
| **Pillar 2 — Toolchain** | | |
| 08 | CMake: The Build System You Can't Avoid | Targets, presets, modules, find_package, cross-compilation, and the modern CMake mental model. |
| 09 | Sanitizers & Debugging: Finding the Unfindable | ASan, TSan, UBSan, LeakSan, GDB, rr, core dumps, and WSL2 constraints. |
| 10 | Profiling & Optimization: Measuring Before Guessing | perf, Valgrind, Cachegrind, SIMD intrinsics, PGO, LTO, and the hierarchy of optimizations that actually matter. |
| 11 | Static Analysis: Catching Bugs Before They Run | clang-tidy, cppcheck, include-what-you-use, and integrating analysis into CI. |
| **Pillar 3 — Systems Programming** | | |
| 12 | OS Fundamentals: What the Kernel Actually Does | Processes, threads, virtual memory, mmap, signals, and the system call interface. |
| 13 | IPC: Making Processes Talk | Pipes, FIFOs, shared memory, message queues, Unix sockets, and choosing the right mechanism. |
| 14 | Low-Level I/O: Talking to Hardware | File descriptors, epoll, io_uring, DMA, memory-mapped I/O, and the async I/O model. |
| 15 | Networking: Sockets from First Principles | TCP/IP stack, BSD sockets, non-blocking I/O, TLS basics, and the C10K problem. |
| **Pillar 4 — Domain Systems** | | |
| 16 | CUDA: GPU Programming from First Principles | Thread hierarchy, memory hierarchy, kernel launches, shared memory, warp divergence, and CUDA streams. |
| 17 | Embedded & RTOS: C++ Without an OS | Bare-metal constraints, FreeRTOS, interrupt service routines, DMA, and MISRA C++ rules. |
| 18 | Robotics Theory: The Math Under ROS2 | SE(3), forward/inverse kinematics, quaternions, the ROS2 compute graph, and real-time constraints. |
| 19 | ROS2: The Full Ecosystem | Nodes, topics, services, actions, parameters, lifecycle, DDS middleware, and production robotics architecture. |
| 20 | AI Inference: Running Models at the Edge | ONNX Runtime, TensorRT, quantization, memory layout for matrix ops, and latency vs throughput tradeoffs. |
| **Pillar 5 — Visual** | | |
| 21 | OpenGL: 3D Graphics from the Ground Up | The rendering pipeline, shaders, VAOs/VBOs, textures, framebuffers, and the GPU memory model. |
| 22 | Qt: Professional Desktop Applications | Signals and slots, the event loop, model/view architecture, QML integration, and cross-platform packaging. |
| 23 | ImGui: Immediate-Mode UI for Tools | The immediate-mode paradigm, backends, custom rendering, and building internal tools engineers actually use. |

---

## The Three-Layer Pyramid

Every chapter in this bible has three layers. You choose how deep to go.

```
        +-----------+
        | Interview |   Targeted Q&A. Practice these the night before.
        +-----+-----+
        | Deep Dive |   Reference-grade. ABI, linkage, mangling, edge cases.
        +-----+-----+
        |   Core    |   The mental model. Read this first. Always.
        +-----------+
```

**Core** (`core.md`): The essential mental model for the chapter domain. Written for someone who has 45 minutes and needs to walk away with a clear picture. No assumed background. Mermaid diagrams for every non-trivial concept. Opinionated — it tells you what actually matters, not what the standard says is possible.

**Deep Dive** (`deep-dive.md`): Reference-grade detail. ABI compatibility, linkage rules, name mangling, implementation internals. Written for the engineer who asked "but why?" after reading Core. This is the layer you return to six months later when you hit a production mystery.

**Interview** (`interview.md`): 8–12 Q&A pairs per chapter, each with a full answer, the wrong answer most candidates give, and the follow-up question that separates seniors from juniors. Sourced from real systems programming interviews.

---

## How Examples Work

Every `examples/` directory contains standalone `.cpp` files. Each file teaches exactly one concept. Every example compiles with:

```bash
g++ -std=c++20 -Wall -Wextra -O2 example.cpp -o example
```

Compiler constraint: GCC 11.4.0. The following C++20 features are available: concepts, coroutines, ranges, `std::span`, `std::jthread`, `std::bit_cast`. The following are **not** available: `std::expected` (GCC 12+), `std::format` (GCC 13+), `std::generator` (GCC 13+).

Examples that require additional flags (e.g., `-lpthread`, `-lGL`) say so in a comment at the top of the file.

---

## Companion Projects

Atlas has built a series of runnable projects in `projects/` that serve as Labs for this tutorial. Each project is a complete, buildable CMake project with real tests.

| Project | Tutorial Lab | Status |
|---------|-------------|--------|
| `projects/01-toolchain` | Chapters 08, 09, 10, 11 | Complete |
| `projects/02-foundation` | Chapters 01–07 | Complete |
| `projects/03-ros2` | Chapters 18, 19 | In progress |

When a chapter's Lab section says "see Atlas's project," navigate to the corresponding `projects/` directory and follow its README. These are real, working implementations — not toy snippets.
