# C++ Senior Interview Prep — Master Roadmap

## Timeline

| Day | Projects | Focus |
|-----|----------|-------|
| Day 1 | 01-toolchain, 02-foundation | CMake mastery + full C++ language core |
| Day 2 | 03-ros2, 04-cuda, 05-embedded, 06-ai-inference | Systems programming |
| Day 3 | 07-opengl, 08-qt, 09-imgui, 10-crown-jewel | Visual/UI + crown jewel |

## Daily Schedule (8–10 hrs)

### Day 1
- 09:00–10:30 CMake presets, sanitizers, flags
- 10:30–11:30 Cross-compilation, LTO/PGO, intrinsics
- 11:30–12:30 clang-tidy, static analysis, assembly output
- 12:30–13:00 Break
- 13:00–14:30 Memory: RAII, smart ptrs, allocators, pmr
- 14:30–16:00 OOP: Rule of 0/3/5, move semantics, CRTP
- 16:00–17:30 Templates: variadic, TMP, SFINAE, concepts
- 17:30–19:00 Rich types, patterns, concurrency, C++20/23

### Day 2
- 09:00–10:30 ROS2: lifecycle nodes, components, params
- 10:30–11:30 ROS2: custom msgs/srvs/actions, TF2, executor
- 11:30–12:00 Break
- 12:00–13:30 CUDA: memory model, thread hierarchy
- 13:30–14:30 CUDA: Thrust, streams, unified memory
- 14:30–15:00 Break
- 15:00–16:30 Embedded: RTOS sim, memory pools, ISR
- 16:30–17:30 Embedded: HSM, ring buffer, MISRA
- 17:30–19:00 AI inference: ONNX/LibTorch C++ API

### Day 3
- 09:00–10:30 OpenGL: VAO/VBO, shaders, lighting, FBO
- 10:30–12:00 Qt: MVC, signals/slots, QML, threading
- 12:00–13:30 ImGui: docking, ImPlot, ImNodes
- 13:30–14:30 Crown jewel: architecture + ECS scaffold
- 14:30–16:00 Crown jewel: CUDA pipeline + OpenGL renderer
- 16:00–17:30 Crown jewel: ImGui panel + coroutine pipeline
- 17:30–18:30 Crown jewel: polish, README, demo

## Implementation Plans

| Plan | File |
|------|------|
| 00 Workspace Bootstrap | `docs/superpowers/plans/2026-05-05-00-workspace-bootstrap.md` |
| 01 Toolchain | `docs/superpowers/plans/2026-05-05-01-toolchain.md` |
| 02 Foundation | `docs/superpowers/plans/2026-05-05-02-foundation.md` |
| 03–10 | Written as each day is reached |

## Success Criteria

- [ ] 10 independent projects built and runnable
- [ ] Every project has a README explaining what it demonstrates
- [ ] Can explain every architectural decision in `10-crown-jewel` from first principles
- [ ] Can reproduce core demos from `02-foundation` live under pressure
- [ ] Can discuss trade-offs without hesitation: virtual vs CRTP, shared_ptr vs arena, exceptions vs expected
- [ ] `01-toolchain` presets work: ASan catches injected bug, TSan catches injected race
