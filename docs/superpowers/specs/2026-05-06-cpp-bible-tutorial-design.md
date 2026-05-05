# C++ Bible Tutorial — Design Spec

**Date:** 2026-05-06 (revised)
**Agent:** Forge (new agent, complement to Atlas)
**Output directory:** `tutorial/`
**Goal:** A complete, self-contained C++ systems engineering encyclopedia — written by a 20-year veteran for their clone. Covers the full language from first principles through every major domain ecosystem. Beginner-friendly entry, expert-depth ceiling.

---

## Context

Atlas (claude-sonnet-4-6) owns the *runnable workspace*: 10 projects built as live demos for interview prep. This tutorial is the **written companion** — the "why, how, and mental model" layer that Atlas's code alone cannot convey.

- Atlas builds: compilable projects, tested code, build infrastructure
- Forge builds: written tutorial, theory, diagrams, embedded examples, interview prep narrative
- Relationship: each tutorial chapter ends with a **Lab** section linking to the relevant Atlas project

The two workstreams are independent but cross-referenced. Neither blocks the other.

---

## Design Decisions

| Decision | Choice | Rationale |
|---|---|---|
| Structure | Two-level pillars + chapters | Mirrors how senior engineers partition expertise; enables targeted lookup |
| Chapter anatomy | Three-layer pyramid (Core / Deep Dive / Interview) | Reader chooses depth; Core = 2-page essentials, Deep Dive = full reference, Interview = ready-to-use answers |
| Code examples | Hybrid — embedded `.cpp` in `examples/` + links to Atlas projects | Self-contained for immediate use; Atlas projects for extended practice |
| Voice | Dual-track — opinionated mentor in narrative, reference-grade in Deep Dive | Reads like a senior talking to a peer; precise when precision matters |
| Domain scope | Full-stack — language core + toolchain + systems programming + domain systems + visual | Clone needs one document for everything |
| Domain depth | Each domain starts from absolute zero — theory, math, tools, ecosystem | No assumed prior knowledge in any domain |
| Systems programming | First-class pillar (Pillar 3) covering OS, IPC, I/O, and networking from kernel theory up | Most common C++ use case; deserves equal standing with language and toolchain |
| Robotics | Split into `18-robotics-theory` (pure theory) + `19-ros2` (implementation) | Robotics theory is a university course; deserves first-class treatment |
| Diagrams | Mermaid blocks in markdown (per CLAUDE.md) | No browser tools; renders in GitHub and most markdown viewers |
| Math | Readable plain-text notation (`x = (-b ± sqrt(b²-4ac)) / 2a`) with plain-English explanation alongside every formula — no LaTeX renderer required | Beginners can read the words; advanced readers can verify the math; renders correctly on GitHub |

---

## Folder Structure

```
tutorial/
│
├── README.md                              ← master index, reading paths, "how to use this bible"
│
├── 00-orientation/                        ← C++ mental model and compilation pipeline
│   ├── README.md
│   ├── core.md
│   ├── deep-dive.md
│   └── interview.md
│
├── pillar-1-language/                     ← pure language mastery (7 chapters)
│   ├── 01-memory/
│   ├── 02-oop/
│   ├── 03-templates/
│   ├── 04-type-system/
│   ├── 05-design-patterns/
│   ├── 06-concurrency/
│   └── 07-modern-cpp/
│
├── pillar-2-toolchain/                    ← build, debug, profile, ship (4 chapters)
│   ├── 08-cmake/
│   ├── 09-sanitizers-debugging/
│   ├── 10-profiling-optimization/
│   └── 11-static-analysis/
│
├── pillar-3-systems-programming/          ← OS, IPC, I/O, networking — from kernel up (4 chapters)
│   ├── 12-os-fundamentals/
│   ├── 13-ipc/
│   ├── 14-low-level-io/
│   └── 15-networking/
│
├── pillar-4-domain-systems/               ← full ecosystem deep dives (5 chapters)
│   ├── 16-cuda/
│   ├── 17-embedded-rtos/
│   ├── 18-robotics-theory/
│   ├── 19-ros2/
│   └── 20-ai-inference/
│
├── pillar-5-visual/                       ← rendering and UI ecosystems (3 chapters)
│   ├── 21-opengl/
│   ├── 22-qt/
│   └── 23-imgui/
│
├── cheatsheets/                           ← printable one-pagers per topic
│   ├── memory-cheatsheet.md
│   ├── concurrency-cheatsheet.md
│   ├── templates-cheatsheet.md
│   ├── cmake-cheatsheet.md
│   ├── sockets-cheatsheet.md
│   ├── ipc-cheatsheet.md
│   ├── cuda-cheatsheet.md
│   ├── embedded-cheatsheet.md
│   ├── robotics-transforms-cheatsheet.md
│   ├── opengl-pipeline-cheatsheet.md
│   └── opengl-math-cheatsheet.md
│
└── interview-master/                      ← cross-cutting interview guide
    ├── README.md
    ├── question-bank.md                   ← 100+ questions with full answers
    ├── system-design-scenarios.md
    ├── live-coding-patterns.md
    └── war-stories.md
```

---

## Chapter Anatomy (applies to every chapter)

```
NN-topic/
├── README.md          ← chapter nav: what you'll learn, prereqs, time estimate, reading paths
├── core.md            ← 2-page essentials — opinionated mentor voice, the 20% that covers 80%
├── deep-dive.md       ← full reference-grade mechanics — every edge case, precise language
├── interview.md       ← questions, gotchas, traps, talking points, answers
└── examples/          ← self-contained .cpp files — each compiles standalone
    ├── 01_<topic>.cpp
    ├── 02_<topic>.cpp
    └── ...
```

Domain chapters (16–23) additionally contain sub-chapter directories for major theory sections, each with the same `core.md / deep-dive.md / interview.md` structure. All code examples for a domain chapter live in the **parent chapter's** `examples/` directory (e.g., `16-cuda/examples/`), not per-sub-chapter — keeps examples discoverable without deep nesting.

---

## Chapter Content Specifications

### `00-orientation` — How C++ Thinks

**core.md:** C++ philosophy (zero-cost abstractions, you don't pay for what you don't use, undefined behavior as a design decision). The compilation pipeline in plain English: source → preprocessor → compiler → assembler → linker → loader → running process.

**deep-dive.md:** Translation units, internal vs external linkage, the One Definition Rule (ODR), name mangling (`c++filt` demo), ABI stability, calling conventions (cdecl, stdcall, System V AMD64), the stack frame layout, how exceptions affect the stack, RTTI overhead.

**interview.md:** "What is undefined behavior and why does C++ have it?" "What's the difference between declaration and definition?" "What happens between `main()` returning and the process exiting?"

---

### `01-memory` — Memory Management

**core.md:** Stack vs heap. RAII as the single most important C++ idiom. `unique_ptr` vs `shared_ptr` vs raw pointer — when to use each. The rule: if you write `delete`, you're doing it wrong.

**deep-dive.md:** Stack unwinding through destructors, `shared_ptr` reference count implementation, `weak_ptr` cycle breaking, custom deleters, placement new, `alignas`/`alignof`, `std::pmr` (monotonic_buffer_resource, pool_resource, custom `memory_resource`), arena allocator internals, memory orders (`seq_cst`, `acquire/release`, `relaxed`) with the happens-before diagram, false sharing and cache line padding.

**examples:** RAII ScopeGuard, arena allocator from scratch, pool allocator, shared_ptr cycle detection, placement new with aligned storage.

**Lab:** `projects/02-foundation/include/foundation/memory/` — raii.hpp, allocators.hpp.

---

### `02-oop` — Object-Oriented Design

**core.md:** Rule of Zero (preferred), Rule of Five (when you manage resources). Move semantics as O(1) ownership transfer. Virtual dispatch as the runtime polymorphism tool; CRTP as the compile-time alternative.

**deep-dive.md:** vtable layout with memory diagram (vptr at offset 0, vtable entries), devirtualization by the compiler, copy-and-swap idiom, the diamond problem and virtual inheritance, Non-Virtual Interface (NVI) pattern, PIMPL for ABI stability, abstract base classes vs concepts for interface design, object slicing and how to prevent it.

**examples:** Rule of Five Buffer class, CRTP counter mixin, NVI reader hierarchy, diamond problem demo, PIMPL Logger.

**Lab:** `projects/02-foundation/include/foundation/oop/` — rules.hpp, crtp.hpp, virtual_design.hpp.

---

### `03-templates` — Templates & Metaprogramming

**core.md:** Templates as compile-time code generation. Function vs class vs variable templates. When to use `auto` return, when to use explicit return type. Concepts (C++20) as the readable face of SFINAE.

**deep-dive.md:** Two-phase name lookup, dependent names (`typename` and `template` disambiguators), full vs partial specialization, variadic templates (recursive vs fold expressions), SFINAE with `enable_if` and `void_t`, tag dispatch, type lists, `constexpr if` vs SFINAE, policy-based design (Alexandrescu), expression templates (lazy evaluation, zero temporaries), concept subsumption rules.

**examples:** TypeList operations, SFINAE has_size trait, policy-based Sorter, expression template Vec arithmetic, variadic tuple-like.

**Lab:** `projects/02-foundation/include/foundation/templates/` — type_traits.hpp, variadic.hpp, concepts.hpp, policy.hpp.

---

### `04-type-system` — The Rich Type System

**core.md:** Strong typedefs prevent `int` aliasing bugs. `std::optional` for nullable values. `std::variant` for type-safe unions. `std::expected` for error-as-value. Use the type system to make illegal states unrepresentable.

**deep-dive.md:** Phantom types for compile-time state machines, `std::variant` implementation internals (aligned storage, discriminant), `overloaded<Fs...>` visitor pattern, monadic operations (`.and_then()`, `.or_else()`, `.transform()`), user-defined literals, `<=>` spaceship operator and its generated comparisons, `explicit` conversion operators and construction, `std::any` internals (small buffer optimization).

**examples:** StrongType<T,Tag> with CRTP ops, PhantomType state machine (Unverified→Verified), variant-based command dispatcher, Expected<T,E> error chain.

**Lab:** `projects/02-foundation/include/foundation/types/` — strong_type.hpp, variant_patterns.hpp, expected.hpp.

---

### `05-design-patterns` — C++ Design Patterns

**core.md:** Which patterns C++ makes unnecessary (garbage collector → RAII), which it makes better (virtual + templates → type erasure), which it makes dangerous (singleton). The patterns you actually use in production: RAII, observer, factory, type erasure.

**deep-dive.md:** Type erasure — the "concept + model" pattern (pedagogical `std::function`), small buffer optimization for type-erased objects. Self-registering factory with static initializers. Thread-safe observer with `std::function`. CRTP decorator for mixin composition. Command pattern with undo/redo using `std::function`. Builder pattern with method chaining. ECS (Entity-Component-System) as a data-oriented design pattern — why it beats OOP for performance-critical systems.

**examples:** AnyCallable type erasure, self-registering shape factory, thread-safe EventEmitter, ECS world with component pools.

**Lab:** `projects/02-foundation/include/foundation/patterns/` — type_erasure.hpp, observer.hpp, factory.hpp.

---

### `06-concurrency` — Concurrent & Parallel Programming

**core.md:** The three problems: data races, deadlocks, and the memory model. The three tools: `mutex` (mutual exclusion), `atomic` (lock-free single variable), `condition_variable` (waiting for state changes). Prefer `std::jthread` (C++20) over `std::thread`.

**deep-dive.md:** The C++ memory model — happens-before, synchronizes-with, the six memory orders with correct and broken examples. Lock-free SPSC queue with acquire/release annotations. Work-stealing thread pool. `std::shared_mutex` for readers-writer lock. `std::counting_semaphore`, `std::latch`, `std::barrier` (C++20). C++20 coroutines: `promise_type`, `awaitable`, `co_await` transformation, `Task<T>` implementation. False sharing and `std::hardware_destructive_interference_size`.

**examples:** SPSC lock-free queue, thread pool with futures, coroutine Task<T>, semaphore-based producer-consumer, broken double-checked locking + correct fix.

**Lab:** `projects/02-foundation/include/foundation/concurrency/` — lock_free_queue.hpp, thread_pool.hpp, coroutine_task.hpp.

---

### `07-modern-cpp` — C++11 Through C++23

**core.md:** What each standard added that changed how you write C++. The features worth learning immediately vs the ones to leave for later.

**deep-dive.md:** Per-standard deep dives:
- C++11: move semantics, lambdas, `auto`, `constexpr`, initializer lists, range-for, smart pointers, `std::thread`
- C++14: generic lambdas, `auto` return deduction, `std::make_unique`, variable templates
- C++17: structured bindings, `if constexpr`, `std::optional/variant/string_view`, fold expressions, CTAD, parallel algorithms, `std::filesystem`
- C++20: concepts, ranges/views, coroutines, modules, `std::span`, three-way comparison, `consteval/constinit`, `std::jthread`, `std::format`
- C++23: `std::expected`, `std::flat_map`, deducing `this`, `std::print`, `std::generator`, `std::stacktrace`

---

### `08-cmake` — Build System Mastery

**core.md:** Targets are the unit of CMake. Everything is `target_*`. Never use directory-level commands. Presets eliminate the "works on my machine" problem.

**deep-dive.md:** Modern target-based CMake (PRIVATE/PUBLIC/INTERFACE), `CMakePresets.json` v3 schema, generator expressions, `find_package` config mode vs module mode, `FetchContent` vs `ExternalProject`, toolchain files for cross-compilation, unity builds, precompiled headers, `cmake --install`, CPack for packaging.

**Ecosystem:** CPM.cmake, Conan 2.0, vcpkg, Hunter — philosophy and tradeoffs of each package manager. Integration with CI (GitHub Actions, GitLab CI). `cmake-format`, `cmake-lint`.

**Lab:** `projects/01-toolchain/CMakePresets.json`, `cmake/modules/`.

---

### `09-sanitizers-debugging` — Finding Bugs Before They Ship

**core.md:** ASan catches use-after-free and buffer overflows at runtime. TSan catches data races. UBSan catches undefined behavior. Run all three in CI. If your tests don't run under sanitizers, they're not really tests.

**deep-dive.md:** How ASan works (shadow memory, red zones), TSan's happens-before graph, UBSan's individual checks (signed overflow, null pointer, misaligned access), MSan for uninitialized reads. Suppressions. Performance overhead tradeoffs.

**Ecosystem:** GDB (breakpoints, watchpoints, backtrace, `info locals`), LLDB, Valgrind/Massif/Helgrind, `rr` (Mozilla's time-travel debugger — record and replay), core dump analysis, DWARF debug info, `addr2line`, `objdump`, `readelf`.

**Lab:** `projects/01-toolchain/` — sanitizer presets and injected-bug demos.

---

### `10-profiling-optimization` — Making It Fast

**core.md:** Measure first, optimize second. The rules: data locality beats algorithms for cache-bound code; branch misprediction costs 15 cycles; a cache miss costs 200. Know your roofline.

**deep-dive.md:** Compiler optimization levels (`-O0` through `-O3`, `-Os`, `-Oz`), LTO (full vs thin), PGO workflow (instrument → run → use), inlining heuristics, loop vectorization (`-fopt-info-vec`), `__builtin_expect` and `[[likely]]`/`[[unlikely]]`, SIMD intrinsics (SSE4.2, AVX2), memory layout for cache efficiency (AoS vs SoA), branch tables vs if-chains.

**Ecosystem:** `perf stat/record/report`, flamegraphs (Brendan Gregg), Cachegrind, Intel VTune, AMD uProf, Tracy profiler (game/realtime), Compiler Explorer (godbolt.org), `std::chrono` microbenchmark harness, Google Benchmark.

**Lab:** `projects/01-toolchain/` — LTO/PGO scripts, AVX2 dot product.

---

### `11-static-analysis` — Automated Code Quality

**core.md:** Static analysis finds bugs without running the code. `clang-tidy` is the Swiss Army knife. Run it in CI as a hard gate.

**deep-dive.md:** `clang-tidy` check categories (modernize, readability, performance, bugprone, cppcoreguidelines), `.clang-tidy` config, suppression comments, `clang-format` style config, `include-what-you-use`, `cppcheck` vs clang-tidy (complementary, not competing).

**Ecosystem:** SonarQube, Coverity, CodeChecker, PVS-Studio, PC-lint, `cpplint`, pre-commit hooks, GitHub Actions integration for automated PR checks.

---

## Pillar 3 — Systems Programming

### `12-os-fundamentals` — Operating Systems from First Principles

Starts from: *what is a process, what does the kernel actually do, how does your program interact with the OS.*

#### Sub-chapters:

**`12a-process-model`**
What a process is: address space, page table, file descriptor table, signal disposition table. Process lifecycle: fork/exec/wait/exit. `fork()` and copy-on-write — diagram showing parent/child address spaces. `exec()` family — replacing the process image. `waitpid()` and zombie/orphan processes. `/proc/<pid>/` — reading process state directly.

**`12b-virtual-memory`**
Physical vs virtual address spaces. The page table (4-level on x86-64) — how the MMU walks it. Page faults — minor (page in from disk) vs major (COW break). `mmap()` — anonymous vs file-backed mappings, MAP_SHARED vs MAP_PRIVATE. Memory protection: `mprotect()`, executable pages, W^X policy. `mlock()` for real-time guarantees. `/proc/<pid>/maps` — reading the memory map. Huge pages (2MB/1GB) and TLB pressure.

**`12c-signals`**
Signal delivery model — asynchronous vs synchronous signals. Signal handlers and async-signal-safe functions (the very short list). `sigaction()` vs `signal()`. `signalfd()` — signals as file descriptors for `select`/`epoll`. `sigprocmask()` for critical sections. Common patterns: `SIGCHLD` reaping, `SIGTERM` graceful shutdown, `SIGSEGV` handler for crash diagnostics.

**`12d-filesystems`**
VFS (Virtual File System) layer — what the kernel does when you call `open()`. Inodes, directory entries, file descriptors. Hard links vs symbolic links — what they are at the inode level. `stat()`/`fstat()` — inode metadata. File locking: `flock()` vs `fcntl()` advisory locks, mandatory locks. Extended attributes (`xattr`). `/proc` and `/sys` as pseudo-filesystems.

**`12e-system-calls`**
What a syscall is: the transition from user space to kernel space (ring 3 → ring 0), `syscall` instruction, the syscall table. Overhead of syscalls — why batching matters. `strace` for tracing syscall activity. VDSO — syscalls that don't enter the kernel (`gettimeofday`, `clock_gettime`). `seccomp` — syscall filtering for sandboxing.

---

### `13-ipc` — Inter-Process Communication

Starts from: *why processes can't share memory by default, and the menu of options the OS gives you.*

#### Sub-chapters:

**`13a-pipes-and-fifos`**
Anonymous pipes: `pipe()`, file descriptor pairs, read/write ends, capacity and blocking behavior. Shell pipelines demystified. Named pipes (FIFOs): `mkfifo()`, persistence in the filesystem, use cases. Pipe capacity and `PIPE_BUF` atomicity guarantee.

**`13b-shared-memory`**
POSIX shared memory: `shm_open()`, `mmap()`, `ftruncate()`. The synchronization problem — shared memory gives you a race condition for free. System V shared memory (`shmget`/`shmat`) — legacy interface, still found in production. Huge page shared memory for low-latency trading systems. Practical pattern: shared memory ring buffer for IPC without copying.

**`13c-message-queues`**
POSIX message queues: `mq_open()`, `mq_send()`, `mq_receive()`, priority ordering, `mq_notify()` for async notification. System V message queues (`msgget`/`msgsnd`/`msgrcv`) — the legacy alternative. When to prefer MQs over pipes: typed messages, priorities, persistence across process restart.

**`13d-unix-domain-sockets`**
Unix domain sockets vs TCP loopback — same API, zero network stack overhead, file-system-based addressing. `SOCK_STREAM` vs `SOCK_DGRAM` vs `SOCK_SEQPACKET`. Credential passing (`SCM_CREDENTIALS`) — how systemd authenticates clients. File descriptor passing (`SCM_RIGHTS`) — sending open file descriptors between processes. Abstract namespace sockets (Linux-only).

**`13e-semaphores-and-mutexes`**
POSIX semaphores: named (`sem_open`) vs unnamed (`sem_init`). Counting semaphore use cases. `pthread_mutex_t` with `PTHREAD_PROCESS_SHARED` attribute for cross-process locking. Futex internals — why `pthread_mutex_t` is fast in the uncontended case. Priority inheritance mutexes for real-time.

---

### `14-low-level-io` — I/O from First Principles

Starts from: *what happens inside the kernel when your program calls read(), and why that matters for performance.*

#### Sub-chapters:

**`14a-io-models`**
The five I/O models (Stevens): blocking, non-blocking, I/O multiplexing, signal-driven, async. Diagram showing user/kernel space for each. Why blocking I/O wastes threads. Why non-blocking I/O needs an event loop. The C10K problem — origin of modern async I/O.

**`14b-io-multiplexing`**
`select()` — the original, O(n) scan, 1024 fd limit. `poll()` — no fd limit, still O(n). `epoll` (Linux) — O(1) with event notification, edge-triggered vs level-triggered, `EPOLLET` subtleties. `kqueue` (BSD/macOS) equivalent. `EPOLLONESHOT` for thread-safe multi-threaded event loops. The thundering herd problem and `EPOLLEXCLUSIVE`.

**`14c-mmap-and-zero-copy`**
`mmap()` for file I/O — reading without `read()` syscalls, page fault on access, write-back on `msync()`. Performance characteristics vs `read()`/`write()` — when mmap wins, when it loses. `sendfile()` — zero-copy file transfer in the kernel. `splice()` and `tee()` — pipe-based zero-copy. `vmsplice()` — user memory into a pipe without copying.

**`14d-io-uring`**
Why `io_uring` was created: eliminating syscall overhead for async I/O. Architecture: submission queue (SQ) + completion queue (CQ) in shared memory — zero-copy between user and kernel. `io_uring_setup()`, `io_uring_enter()`. Fixed buffers and registered files — further reducing overhead. `liburing` — the ergonomic C API. Probes: what operations are supported. Real benchmark: throughput vs epoll for file + network I/O.

**`14e-memory-mapped-files-and-dma`**
`madvise()` hints (`MADV_SEQUENTIAL`, `MADV_RANDOM`, `MADV_WILLNEED`, `MADV_DONTNEED`). Huge pages for mmap regions. `posix_fadvise()` for read-ahead hints. DMA (Direct Memory Access) concept — how hardware bypasses the CPU. `O_DIRECT` for bypassing the page cache. `O_SYNC`/`O_DSYNC` for durability guarantees.

---

### `15-networking` — Network Programming from First Principles

Starts from: *what happens at the hardware level when you send a packet, and how the TCP/IP stack turns bytes into reliable streams.*

#### Sub-chapters:

**`15a-network-theory`**
OSI model — what each layer does and which layer you code at. Physical → Data Link → Network → Transport → Application. Ethernet frames, IP packets, TCP/UDP segments — header layouts with field-by-field explanation. ARP — how IP addresses map to MAC addresses. Routing — how a packet finds its way across the internet. NAT — why your home router works the way it does.

**`15b-tcp-ip-deep-dive`**
TCP three-way handshake (SYN/SYN-ACK/ACK) and four-way close. Sequence numbers and acknowledgment — reliable delivery mechanism. Sliding window — flow control. Congestion control: slow start, congestion avoidance, fast retransmit, fast recovery (Reno, CUBIC, BBR). Nagle algorithm and `TCP_NODELAY`. `TIME_WAIT` state — why it exists and how to manage it. TCP_QUICKACK for low-latency.

**`15c-bsd-sockets`**
Socket API from scratch: `socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `send()`/`recv()`, `close()`. IPv4 vs IPv6 — `sockaddr_in` vs `sockaddr_in6`, `getaddrinfo()` for protocol-agnostic code. `SO_REUSEADDR`/`SO_REUSEPORT` — what they actually do. `setsockopt()` options that matter in production: `SO_KEEPALIVE`, `TCP_KEEPIDLE`, `SO_SNDBUF`/`SO_RCVBUF`, `SO_LINGER`. UDP sockets — `sendto()`/`recvfrom()`, broadcast, multicast.

**`15d-async-and-event-driven`**
Non-blocking sockets + `epoll` — the reactor pattern. Proactor pattern vs reactor. `Boost.Asio` — `io_context`, async operations, strands for thread safety, coroutine integration (`co_await`). `libuv` — Node.js's event loop in C. `libevent`/`libev` — older but still deployed. io_uring for networking — submission-based async send/recv.

**`15e-high-performance-networking`**
Kernel bypass: why the kernel is the bottleneck at 10Gbps+. **DPDK** (Data Plane Development Kit) — poll-mode drivers, huge page memory, lockless ring buffers, packet processing without kernel. **RDMA** (Remote Direct Memory Access) — InfiniBand and RoCE, one-sided operations (read/write without remote CPU), `libibverbs` API. **XDP** (eXpress Data Path) — eBPF programs in the NIC driver, before the kernel network stack. Zero-copy networking patterns.

**`15f-protocols-and-serialization`**
Protocol design: framing (length-prefix vs delimiter), endianness (`htonl`/`ntohl`), versioning. **Protocol Buffers** (protobuf) — schema definition, generated C++ code, binary encoding. **FlatBuffers** — zero-copy deserialization, when to use over protobuf. **MessagePack** — binary JSON. **Cap'n Proto** — zero-copy like FlatBuffers with RPC. **gRPC** — HTTP/2 + protobuf, C++ client/server, streaming RPCs. TLS with **OpenSSL** and **mbedTLS** — TLS handshake, certificates, `BIO` abstraction.

**`15g-ecosystem`**
**Boost.Asio** (async I/O and networking), **libuv** (cross-platform async), **ZeroMQ** (brokerless messaging — PUB/SUB, PUSH/PULL, REQ/REP), **NNG** (nanomsg next gen), **NATS** (cloud-native messaging), **Cap'n Proto RPC**, **gRPC**, **Wireshark** (packet capture and analysis), **tcpdump**, **netstat**/**ss**, **iperf3** (bandwidth testing), **wrk**/**hey** (HTTP benchmarking).

---

## Pillar 4 — Domain Systems

### `16-cuda` — GPU Programming from First Principles

Starts from: *what is a GPU, why it exists, how it differs from a CPU at the silicon level.*

#### Sub-chapters:

**`16a-gpu-architecture`**
Theory: Flynn's taxonomy (SISD/SIMD/MIMD), why GPUs have thousands of cores, SM (Streaming Multiprocessor) internals, warp execution model (32 threads in lockstep), warp scheduler, register file, L1/L2/DRAM hierarchy. Roofline model diagram. Memory bandwidth vs compute bound analysis.
Math: Occupancy formula `(active warps per SM) / (max warps per SM)`, memory bandwidth calculation, Amdahl's law for GPU speedup prediction.

**`16b-cuda-programming-model`**
Thread/block/grid hierarchy with 3D diagrams. Kernel launch syntax. Shared memory as user-managed L1. `__syncthreads()`. Atomic operations. Warp-level primitives (`__shfl_xor_sync`, `__ballot_sync`). Divergence and its cost.

**`16c-memory-patterns`**
Global memory coalescing rules with access pattern diagrams. Bank conflicts in shared memory — how they occur, how to avoid them (padding, shuffle). Constant memory for broadcast reads. Texture memory for spatial locality. Unified memory (`cudaMallocManaged`) and prefetch hints.

**`16d-streams-and-async`**
Default stream vs named streams. Concurrent kernel execution. H2D/D2H overlap with compute. Events for timing and synchronization. Multi-GPU with `cudaSetDevice`, peer access, NVLink topology.

**`16e-ecosystem`**
**Thrust** (GPU STL — `device_vector`, `transform`, `reduce`, `sort`), **cuBLAS** (batched GEMM), **cuFFT**, **cuDNN** (tensor operations for deep learning), **cuSPARSE**, **NCCL** (multi-GPU collective comms), **TensorRT** (inference graph optimization), **CUDA-GDB**, **Nsight Systems** (system-level timeline), **Nsight Compute** (kernel-level metrics), **nvtx** range annotations.

**Lab:** `projects/04-cuda/`.

---

### `17-embedded-rtos` — Embedded Systems from First Principles

Starts from: *what is a microcontroller, what "bare metal" means, why embedded is different.*

#### Sub-chapters:

**`17a-hardware-fundamentals`**
Von Neumann vs Harvard architecture. Memory-mapped I/O — how writing to an address controls hardware. Interrupt controller (NVIC on ARM Cortex-M). Clock tree, prescalers, PLL. Linker scripts: `.text`, `.data`, `.bss`, vector table. Startup code (`Reset_Handler`). Stack/heap placement in MCU memory map.
Math: Fixed-point arithmetic (Q format), PWM duty cycle, ADC resolution, Nyquist theorem.

**`17b-rtos-theory`**
Cooperative vs preemptive scheduling. Priority levels. Context switch mechanics (saving/restoring registers). Priority inversion and priority inheritance/ceiling protocol. Deadlock conditions (Coffman). Jitter and worst-case execution time (WCET). Tickless idle mode. Memory allocation in RTOS — why `malloc` is forbidden.

**`17c-rtos-ecosystem`**
**FreeRTOS** (most widely used — tasks, queues, semaphores, mutexes, event groups, stream buffers), **Zephyr** (Linux Foundation, devicetree, west tool, modern C API), **ThreadX/Azure RTOS** (Microsoft, safety-certified), **RTEMS** (space/aviation), **ChibiOS** (automotive).
Tradeoffs table: footprint, safety certification, community, tooling.

**`17d-hardware-tools`**
JTAG vs SWD debugging. **OpenOCD** — connecting to hardware, GDB server. **J-Link** and **ST-Link** probes. Logic analyzers for I2C/SPI/UART/CAN protocol capture. Oscilloscopes for analog signals. **Segger SystemView** for RTOS task tracing.

**`17e-safety-patterns`**
MISRA C++ 2023 key rules — no dynamic allocation, no recursion, bounded loops, explicit casts. **CERT C++** secure coding. Defensive programming for embedded: watchdog timers, stack canaries, CRC checks on flash. Safety standards: IEC 61508 (functional safety), ISO 26262 (automotive ASIL A-D), DO-178C (aviation). Tools: **Polyspace**, **PC-lint Plus**, **Parasoft C/C++test**.

**Lab:** `projects/05-embedded/`.

---

### `18-robotics-theory` — Robotics from First Principles

Starts from: *what is a robot, what does it mean to control a physical system, why math governs everything.*

#### Sub-chapters:

**`18a-coordinate-frames`**
What a coordinate frame is and why robots need many of them. Rotation matrices — derived from 2D rotation, extended to 3D. Euler angles (roll/pitch/yaw) and their gimbal lock problem. Quaternions — unit quaternion as rotation, `q = w + xi + yj + zk`, SLERP interpolation. Homogeneous transforms (4×4 SE(3) matrix) — composing position + rotation in one operation.
Mermaid diagram: frame tree for a 6-DOF robot arm.

**`18b-kinematics`**
Forward kinematics: given joint angles, where is the end-effector? Denavit-Hartenberg (DH) parameters — four parameters per joint, systematic frame assignment rules. Step-by-step DH table for a 2-link planar arm with worked example.
Inverse kinematics: geometric approach for simple chains; numerical methods (Newton-Raphson, Jacobian pseudoinverse) for general case. Singularities — when the Jacobian loses rank, what it means physically.
Jacobian matrix: velocity kinematics, force/torque duality.

**`18c-control-theory`**
Open-loop vs closed-loop — diagram showing plant, controller, sensor, feedback path. Why open-loop fails in the real world.
PID controller: P (proportional — responds to error), I (integral — eliminates steady-state error), D (derivative — dampens oscillation). Intuitive explanation + step response diagrams. Tuning: Ziegler-Nichols rules. Anti-windup for the integrator.
State-space representation: `ẋ = Ax + Bu`, `y = Cx + Du`. What A, B, C, D mean physically. Poles and eigenvalues — where poles are in the complex plane determines stability.
Stability: Lyapunov's second method — energy-like function that decreases. BIBO stability.
LQR (Linear Quadratic Regulator): minimize `∫(xᵀQx + uᵀRu)dt`. Cost matrix intuition (Q penalizes state error, R penalizes control effort). Solving Riccati equation.
MPC (Model Predictive Control): predict future states over horizon N, optimize, apply first control input, recede. Why it handles constraints naturally.

**`18d-perception-and-sensor-fusion`**
Sensor types and noise models: LiDAR (time-of-flight, range noise), IMU (accelerometer + gyroscope, bias, drift), camera (projection model, lens distortion), GPS (position, multipath), wheel encoders (quantization error).
Probabilistic state estimation: representing state as a Gaussian `p(x) = N(μ, Σ)`.
**Kalman Filter** from scratch: predict step (`μ_pred = Aμ + Bu`, `Σ_pred = AΣAᵀ + Q`), update step (Kalman gain, innovation). 1D example worked in detail.
**Extended Kalman Filter** (EKF): nonlinear `f(x)` and `h(x)`, Jacobian linearization at operating point.
**Particle Filter**: Monte Carlo localization — particles represent distribution, importance sampling, resampling. When to use vs KF (non-Gaussian, multi-modal distributions).
IMU preintegration: why we don't integrate raw IMU directly, preintegration on manifold.

**`18e-slam`**
The SLAM problem: chicken-and-egg — need map to localize, need pose to map. Why it's hard.
Occupancy grids: probabilistic cell occupancy, log-odds update rule.
EKF-SLAM: landmarks as state vector entries, scaling problem (O(n²)).
Graph-based SLAM: pose graph nodes (robot poses) + edges (constraints from sensor measurements). Loop closure as graph optimization (g2o, GTSAM).
Modern SLAM systems overview: **ORB-SLAM3** (visual/visual-inertial, feature-based), **RTAB-Map** (RGB-D, graph-based, long-term), **Cartographer** (Google, 2D/3D lidar), **LIO-SAM** (lidar-inertial), **LOAM** variants.
Point cloud processing: voxel grid downsampling, **ICP** (Iterative Closest Point) — point-to-point vs point-to-plane, convergence conditions. Normal Distributions Transform (NDT).

**`18f-path-planning`**
Configuration space (C-space): why we plan in joint/config space, obstacle inflation, 2D C-space visualization for a 2-link arm.
Graph search: **Dijkstra** (optimal, exhaustive), **A\*** (heuristic-guided, completeness + optimality conditions), admissible heuristics. Step-by-step grid example with diagrams.
Sampling-based: **RRT** (rapidly-exploring random tree) — algorithm, probabilistic completeness. **RRT\*** — asymptotic optimality, rewiring step. **PRM** (probabilistic roadmap) for multi-query.
Potential fields: attractive potential toward goal, repulsive from obstacles. Local minima problem and escapes (random walk, navigation functions).
Trajectory optimization: minimum-snap polynomial trajectories for quadrotors. Velocity/acceleration constraints, differential drive kinematics vs holonomic.

**`18g-advanced-robotics`**
Dynamics: Newton-Euler recursive algorithm, Lagrangian mechanics for serial manipulators. Inertia tensor.
Force control: impedance control (desired mechanical impedance between robot and environment), admittance control (dual formulation).
Behavior trees: nodes (sequence, selector, parallel, decorator, leaf), advantages over state machines for complex tasks, **BehaviorTree.CPP** library.
Multi-robot: formation control (consensus algorithms), task allocation (auction-based, Hungarian algorithm).
Sim-to-real: domain randomization, system identification, why `gazebo_physics ≠ real_physics`.

**Lab:** `projects/03-ros2/` (Nav2 stack, TF2 usage).

---

### `19-ros2` — Robot Operating System 2

Starts from: *what is middleware, why robots need it, what problem ROS2 solves.*

**Core concepts:** Node, topic (pub/sub), service (req/res), action (long-running with feedback), parameter system, lifecycle nodes, composable components, executors (single-threaded, multi-threaded, static-single), launch system.

**DDS theory:** RTPS protocol, QoS policies (reliability, durability, deadline, liveliness), participant discovery, vendor comparison: **FastDDS** (default), **CycloneDDS** (Eclipse), **Connext DDS** (RTI, commercial).

**Ecosystem:**
- **Nav2**: behavior tree-based navigation, costmaps (global/local), planners (NavFn, Smac), controllers (DWB, RPP), recovery behaviors
- **MoveIt2**: motion planning pipeline, planning scene, OMPL planners, collision checking, IKFast/KDL solvers
- **ros2_control**: hardware abstraction layer, controller manager, joint trajectory controller
- **Foxglove Studio**: visualization and data inspection, panels, layout, mcap format
- **RViz2**: 3D visualization, custom displays, interactive markers
- **rosbag2**: recording/playback, mcap format, SQLite backend, filtering and conversion
- **Gazebo/Ignition**: physics simulation, sensor plugins, ROS2 bridge
- **ros2_tracing + LTTng**: system-level performance tracing

**Lab:** `projects/03-ros2/`.

---

### `20-ai-inference` — AI/ML Inference from First Principles

Starts from: *what is a neural network at the math level — weights, activations, forward pass.*

#### Sub-chapters:

**`20a-neural-network-theory`**
Perceptron → MLP → CNN → Transformer. Forward pass math. Matrix multiply as the universal building block. Convolution as a sliding dot product — intuition with a 3×3 kernel on a 5×5 image. Attention mechanism: `Q·Kᵀ / sqrt(d)` → softmax → weighted V. Inference vs training distinction — why inference is simpler but still hard to optimize.

**`20b-quantization-and-formats`**
Floating point formats: FP32, FP16, BF16, INT8 — precision vs range tradeoffs. Post-training quantization (PTQ) vs quantization-aware training (QAT). Symmetric vs asymmetric quantization, scale factors, zero-points. Model exchange formats: ONNX (universal), TorchScript (PyTorch portable), CoreML (Apple), TFLite (mobile). Graph optimization: operator fusion, constant folding, dead code elimination.

**`20c-inference-engines`**
**ONNX Runtime** (cross-platform — CPU, CUDA, TensorRT, OpenVINO EPs), **LibTorch/TorchScript** (PyTorch C++ API), **TensorRT** (NVIDIA — kernel auto-tuning, FP16/INT8, dynamic shapes), **OpenVINO** (Intel CPU/iGPU/VPU), **TFLite** (mobile/edge), **llama.cpp** (quantized LLM inference on CPU).

**`20d-pipeline-and-deployment`**
Pre/post-processing pipeline in C++: image decode, resize, normalize, tensor pack. Memory planning: pre-allocated inference buffers, zero-copy where possible. Dynamic batching with memory reuse across frames. p50/p95/p99 latency harness. **Triton Inference Server** — model repository, versioning, concurrent model execution, gRPC/HTTP endpoints. NVIDIA Container Toolkit for GPU containerization.

**Lab:** `projects/06-ai-inference/`.

---

## Pillar 5 — Visual

### `21-opengl` — Computer Graphics from First Principles

Starts from: *what is a pixel, what is a frame, what is the rendering pipeline.*

#### Sub-chapters:

**`21a-graphics-theory`**
What is rendering: converting a 3D scene description into a 2D image. Rasterization vs ray tracing — tradeoffs. The human visual system and color perception. Color spaces: linear light vs sRGB (gamma 2.2). The pinhole camera model.
The classic rasterization pipeline diagram: Application → Vertex Processing → Primitive Assembly → Rasterization → Fragment Processing → Output Merger.

**`21b-linear-algebra-for-graphics`**
Vectors: dot product (projection, angle), cross product (perpendicular vector, winding order). Matrices as linear transforms. Matrix multiplication as transform composition — order matters. Homogeneous coordinates — why we add a 4th component. Model matrix (object→world), View matrix (world→camera), Projection matrix (camera→clip). Perspective divide and NDC. Viewport transform to screen pixels.
All matrices derived from first principles with diagrams.

**`21c-modern-opengl`**
Core profile (no legacy). VAO/VBO/EBO — what each stores and why the order of binding matters. GLSL shader pipeline: vertex shader (transforms vertices), fragment shader (colors pixels). Shader program compilation and linking. Uniform variables. Uniform Buffer Objects (UBO) for shared data. Texture objects, samplers, mipmaps, filtering modes. Framebuffer Objects (FBO) for off-screen rendering.

**`21d-lighting-and-shading`**
Phong lighting model: ambient (base illumination) + diffuse (Lambertian, `dot(N, L)`) + specular (Phong reflection, `dot(R, V)^shininess`). Blinn-Phong (half-vector optimization). Normal mapping — encoding surface normals in a texture. Shadow mapping — render depth from light, compare in main pass.
PBR (Physically Based Rendering): metallic-roughness workflow, Cook-Torrance BRDF (`D × F × G / (4 × NdotL × NdotV)`), energy conservation, image-based lighting (IBL).

**`21e-advanced-rendering`**
Deferred rendering — why: decouple geometry from lighting, handle many lights. G-buffer layout. SSAO (Screen-Space Ambient Occlusion). HDR rendering + tone mapping (Reinhard, ACES). Instanced rendering for thousands of objects. Geometry shaders. Compute shaders. CUDA-OpenGL interop (`cudaGraphicsGLRegisterBuffer`).

**`21f-ecosystem`**
**GLFW** (windowing + input), **GLAD/GLEW** (extension loading), **GLM** (GLSL-compatible math library), **stb_image** (texture loading), **Assimp** (model loading — OBJ, FBX, GLTF), **RenderDoc** (frame capture + GPU debugger), **Nsight Graphics** (NVIDIA profiler), **Vulkan** — when and why to move from OpenGL (explicit control, multi-threading, no global state), **WebGL/WebGPU** comparison.

**Lab:** `projects/07-opengl/`.

---

### `22-qt` — GUI Application Development from First Principles

Starts from: *what is an event loop, what is a widget, how GUIs work at the OS level.*

**Theory:** Event-driven programming, the message pump loop, platform abstraction (WinAPI/Cocoa/X11/Wayland under Qt). Retained mode GUI — widget tree owns its representation. Why Qt chose MOC (meta-object compiler) over pure C++ for signals/slots.

**Core Qt:** QObject tree and memory management, QWidget hierarchy and layout managers, signals & slots (MOC-generated code, connection types: direct/queued/blocking-queued), model/view/delegate architecture (`QAbstractItemModel` — separation of data from presentation), event handling (`QEvent` subclasses, `event()` vs `paintEvent()`/`mousePressEvent()`).

**QML:** Declarative UI syntax, property binding engine, JavaScript integration, C++/QML bridge (`Q_PROPERTY`, `QML_ELEMENT`, `QQmlContext`), `QtQuick.Controls 2` styled components, state machines and transitions, animations.

**Threading:** `QThread` (subclass vs worker-object pattern), `QtConcurrent::run` and `QFuture`/`QPromise`, thread-safe signal emission across thread boundary (queued connection), `QThreadPool`.

**Ecosystem:** Qt Creator IDE, Qt Designer, `qmake` vs CMake (prefer CMake), Qt Charts, Qt Data Visualization (3D), Qt Network (`QNetworkAccessManager`, REST, WebSockets), Qt Multimedia, Qt3D, Qt WebEngine (Chromium embedded), Qt Installer Framework, `QTest` unit testing framework, `lupdate`/`lrelease` for i18n, `windeployqt`/`macdeployqt` for deployment.

**Lab:** `projects/08-qt/`.

---

### `23-imgui` — Immediate Mode GUI from First Principles

Starts from: *retained vs immediate mode — the philosophical difference and when immediate wins.*

**Theory:** Retained mode (Qt, browser DOM): framework owns widget tree, you mutate it. Immediate mode (ImGui): you redraw everything every frame, framework keeps no state. Why games and tools prefer immediate: no state synchronization bugs, UI code reads like imperative logic, trivially embeds debug info.

**Core ImGui:** Context and IO structure. The draw list model (CPU-side vertex buffers submitted to GPU). Widget ID system and the ID stack — how `PushID`/`PopID` prevents widget collisions. Window flags. Tables API (C++20 style, replaces Columns). Child windows. Modal and popup management.

**Backends:** How the OpenGL3+GLFW backend works (font atlas as texture, draw list to GL calls). SDL2+OpenGL. Vulkan backend. DirectX backends. Writing a minimal custom backend from scratch.

**Ecosystem:**
- **ImPlot**: scientific plotting (line, scatter, bar, heatmap, real-time scrolling), axis configuration, custom ticks
- **ImNodes**: node-based graph editor (pins, links, minimap) for pipeline visualization
- **ImGuizmo**: 3D manipulation gizmos (translate/rotate/scale handles) in world space
- **imgui-filebrowser**: cross-platform file dialog
- Docking branch: `ImGuiConfigFlags_DockingEnable`, persistent layout serialization
- Viewports: multi-window support beyond the host window

**Patterns:** Debug overlay pattern, settings panel pattern, per-frame profiler panel, INI serialization of UI state.

**Lab:** `projects/09-imgui/`.

---

### `cheatsheets/`

One-page quick references — printable or pinned in a second monitor. Topics: memory, concurrency, templates, cmake, sockets API, IPC mechanisms comparison, epoll patterns, cuda memory model, embedded RTOS tasks/queues, robotics transforms, OpenGL pipeline stages, OpenGL math, ImGui widget API.

---

### `interview-master/`

- **question-bank.md:** 100+ questions organized by chapter, each with a full answer, the trap answer, and a follow-up. Format: `Q:` / `A:` / `Trap:` / `Follow-up:`
- **system-design-scenarios.md:** 15 system design prompts with worked solutions — "design a lock-free queue," "design a high-performance TCP server," "design a ROS2 navigation stack," "design a real-time rendering pipeline," "design an embedded RTOS scheduler"
- **live-coding-patterns.md:** 25 patterns that appear on C++ whiteboards — RAII wrapper, pairwise swap, shared_ptr from scratch, producer-consumer, ring buffer, epoll echo server
- **war-stories.md:** 5 narrative templates for "tell me about the most complex system you've built" — framed around the Atlas crown jewel and domain projects

---

## Agent Identity: Forge

**Name:** Forge
**Model:** claude-sonnet-4-6
**Role:** Tutorial author and knowledge architect
**Mission:** Write the C++ bible — the written counterpart to Atlas's runnable workspace. Every concept explained from first principles, every domain covered as a full ecosystem.

**Collaboration with Atlas:**
- I read Atlas's log before starting any chapter — if Atlas is actively building a project, I do not start the corresponding Lab section until the project is committed
- I do not modify Atlas's project files — I only link to them from tutorial Lab sections
- I drop handoff notes in `.agents/incoming/` when I need Atlas to add something to a project for a tutorial example
- I create `.agents/forge/identity.md` and `.agents/forge/log.md` before any tutorial work begins

---

## Coordination with Atlas

| Tutorial Chapter | Atlas Project | Status |
|---|---|---|
| `01-memory` through `06-concurrency` | `02-foundation` | Atlas: DONE ✓ — can write now |
| `08-cmake` | `01-toolchain` | Atlas: DONE ✓ — can write now |
| `09-sanitizers-debugging` | `01-toolchain` | Atlas: DONE ✓ — can write now |
| `10-profiling-optimization` | `01-toolchain` | Atlas: DONE ✓ — can write now |
| `12-os-fundamentals` through `15-networking` | (no Atlas project — Forge-only) | No Atlas dependency |
| `16-cuda` | `04-cuda` | Atlas: NOT STARTED — write theory/ecosystem; hold Lab section |
| `17-embedded-rtos` | `05-embedded` | Atlas: NOT STARTED — write theory/ecosystem; hold Lab section |
| `18-robotics-theory` | (theory only, no Atlas project) | No Atlas dependency |
| `19-ros2` | `03-ros2` | Atlas: NOT STARTED — write theory/ecosystem; hold Lab section |
| `20-ai-inference` | `06-ai-inference` | Atlas: NOT STARTED — write theory/ecosystem; hold Lab section |
| `21-opengl` | `07-opengl` | Atlas: NOT STARTED — write theory/ecosystem; hold Lab section |
| `22-qt` | `08-qt` | Atlas: NOT STARTED — write theory/ecosystem; hold Lab section |
| `23-imgui` | `09-imgui` | Atlas: NOT STARTED — write theory/ecosystem; hold Lab section |

---

## Chapter Count

| Pillar | Chapters | Numbers |
|---|---|---|
| Orientation | 1 | 00 |
| Pillar 1 — Language | 7 | 01–07 |
| Pillar 2 — Toolchain | 4 | 08–11 |
| Pillar 3 — Systems Programming | 4 | 12–15 |
| Pillar 4 — Domain Systems | 5 | 16–20 |
| Pillar 5 — Visual | 3 | 21–23 |
| **Total** | **24** | |

---

## Success Criteria

- [ ] `tutorial/README.md` exists with full navigation index and reading paths
- [ ] Every chapter has `README.md`, `core.md`, `deep-dive.md`, `interview.md`
- [ ] Every chapter has at least 3 self-contained compilable examples in `examples/`
- [ ] Systems programming chapters (12–15) cover OS, IPC, I/O, and networking from kernel theory up
- [ ] Every domain chapter (16–23) has sub-chapter structure with theory from zero
- [ ] `interview-master/question-bank.md` contains 100+ questions with full answers
- [ ] All diagrams are Mermaid blocks inside markdown (no browser tools, per CLAUDE.md)
- [ ] Math is readable plain-text notation with plain-English explanation alongside
- [ ] Cheatsheets exist for all major topics
- [ ] Forge identity and log files exist under `.agents/forge/`
- [ ] No tutorial file modifies anything under `projects/` (Atlas's domain)

---

## Out of Scope

- Building or modifying any code under `projects/` — that is Atlas's responsibility
- Generating images or binary assets — diagrams are Mermaid in markdown only
- Writing a full CUDA/ROS2/OpenGL tutorial that duplicates what Atlas's project READMEs already explain — the tutorial covers theory and ecosystem; Atlas's READMEs cover the specific project implementation
