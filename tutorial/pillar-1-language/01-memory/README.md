# Chapter 01: Memory — The Foundation of Everything

Memory is the lens through which C++ must be understood. Every performance characteristic, every safety concern, every design decision ultimately comes back to how memory is acquired, used, and released. This chapter teaches you to think about memory the way the machine does — and then shows you the C++ tools that make it manageable at scale.

**Prerequisites:** Chapter 00-orientation (the compilation pipeline and UB mental model).

**Time estimate:** Core = 2 hours · Deep Dive = 3 hours · Interview = 30 minutes

**Reading paths:**
- "I just need to stop leaking memory" → `core.md` (The Golden Rule + Smart Pointer Decision Tree)
- "I'm designing a high-performance allocator" → `deep-dive.md` (Arena Allocator Internals + std::pmr)
- "Interview tomorrow" → `interview.md` (all 8 Q&A pairs)
- "Show me running code" → `examples/` (3 programs: ScopeGuard, smart pointers, arena)

**Atlas Lab:** `projects/02-foundation/include/foundation/memory/` — production-grade RAII, arena, pool, and stack allocators. Run: `cmake --preset debug && cmake --build --preset debug && ctest --preset debug` from `projects/02-foundation/`.
