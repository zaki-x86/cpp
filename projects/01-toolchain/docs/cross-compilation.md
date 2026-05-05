# Cross-Compilation for ARM Targets

This document covers the cross-compilation toolchain configurations in the
`01-toolchain` project: an ARM Linux target (for SBCs and embedded Linux boards)
and an ARM bare-metal target (for microcontrollers with no OS). Both toolchain
files live in `toolchain/` and are wired into `CMakePresets.json`.

---

## Table of Contents

1. [What Cross-Compilation Is and Why It Matters](#what-cross-compilation-is-and-why-it-matters)
2. [Target Triple Anatomy](#target-triple-anatomy)
3. [ARM Linux (gnueabihf) Toolchain](#arm-linux-gnueabihf-toolchain)
4. [ARM Bare-Metal (none-eabi) Toolchain](#arm-bare-metal-none-eabi-toolchain)
5. [CMake Presets for Cross-Compilation](#cmake-presets-for-cross-compilation)
6. [How to Invoke Each Toolchain](#how-to-invoke-each-toolchain)
7. [Linker Script Considerations for Bare-Metal](#linker-script-considerations-for-bare-metal)
8. [Interview Talking Points](#interview-talking-points)

---

## What Cross-Compilation Is and Why It Matters

**Cross-compilation** means building code on one machine (the *host*) for a different
machine (the *target*). The host and target differ in at least one of:

- CPU architecture (x86-64 host → ARM target)
- Operating system (Linux host → bare-metal target with no OS)
- ABI (application binary interface, including calling convention and floating-point handling)

### Why It Matters in Interviews

Almost every embedded, robotics, automotive, and IoT role involves cross-compilation:

- **Robotics** (ROS 2): Packages are often developed on x86 workstations but deployed to
  ARM SBCs (Raspberry Pi, NVIDIA Jetson, BeagleBone).
- **Embedded / firmware**: Microcontrollers (Cortex-M, RISC-V) cannot run a host OS;
  all code is compiled on a workstation and flashed.
- **Automotive / safety-critical**: Production ECUs often run QNX or bare AUTOSAR on
  Cortex-R/A cores; development happens on Linux x86 hosts.

Being able to set up and reason about a cross-compilation toolchain — including sysroot
management, ABI flags, and what happens when `find_library()` accidentally picks up a
host library — is a meaningful differentiator.

---

## Target Triple Anatomy

The **target triple** (actually a quadruple in some cases) encodes:

```
<arch>-<vendor>-<os>-<environment>

arm   - linux  - gnueabihf    ← ARM Linux, hard-float
arm   - none   - eabi         ← ARM bare-metal, soft/hard-float ABI base
```

| Field | arm-linux-gnueabihf | arm-none-eabi |
|-------|---------------------|---------------|
| arch | `arm` (32-bit ARMv7) | `arm` (32-bit ARM) |
| vendor | *(implicit: unknown)* | *(implicit: unknown)* |
| os | `linux` (Linux kernel + glibc) | `none` (no OS) |
| environment | `gnueabihf` (GNU libc, hard-float) | `eabi` (Embedded ABI) |

**`gnueabihf` decoded:**

- `gnu` — uses glibc (GNU C Library) as the C runtime
- `eabi` — ARM Embedded ABI (defines register usage, stack layout, calling conventions)
- `hf` — **hard-float**: the FPU executes floating-point instructions; arguments are
  passed in FPU registers (VFP/NEON). Contrast with `gnueabi` (soft-float) where the
  compiler emits software float emulation calls and passes float arguments in integer
  registers.

The `hf` distinction has a real binary compatibility impact: a library compiled for
`gnueabihf` cannot be linked into a `gnueabi` binary and vice versa.

---

## ARM Linux (gnueabihf) Toolchain

**File:** `toolchain/arm-linux-gnueabihf.cmake`

This toolchain targets Linux-based ARM boards running a full userland (Raspberry Pi,
BeagleBone, i.MX6, etc.).

### Installation

```bash
sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

### Full Annotated File

```cmake
# Tell CMake we are cross-compiling for a Linux target.
# This suppresses the assumption that the target == the host.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# The prefix shared by all binaries in the toolchain.
set(TOOLCHAIN_PREFIX arm-linux-gnueabihf)

# find_program() searches PATH for these names.
# Using find_program() instead of set() means CMake errors clearly
# if the cross-toolchain is not installed rather than failing later.
find_program(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
find_program(CMAKE_AR           ${TOOLCHAIN_PREFIX}-ar)    # Cross archiver
find_program(CMAKE_STRIP        ${TOOLCHAIN_PREFIX}-strip)  # Cross strip

# Passed to the compiler as --target= (Clang) or advisory (GCC).
set(CMAKE_C_COMPILER_TARGET   ${TOOLCHAIN_PREFIX})
set(CMAKE_CXX_COMPILER_TARGET ${TOOLCHAIN_PREFIX})

# Sysroot: the tree of headers and libraries for the target.
# /usr/arm-linux-gnueabihf/ is populated by the apt package above.
# It contains the ARM glibc, libstdc++, and system headers.
set(CMAKE_SYSROOT /usr/${TOOLCHAIN_PREFIX})
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

# Search modes: controls where CMake's find_*() commands look.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Programs (cmake tools, scripts) are always host binaries → never search sysroot.

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# Libraries must come from the sysroot — NEVER from the host's /usr/lib.
# Mixing host and target libraries causes linker errors or silent ABI mismatches.

set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# Headers must come from the sysroot. The host's /usr/include contains
# x86-specific definitions (e.g., <bits/...>) that would be wrong for ARM.

set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
# CMake package config files must be found under the sysroot as well.

# Compiler flags applied to every source file.
add_compile_options(
    -marm           # Emit 32-bit ARM instructions (not Thumb)
    -mfpu=neon      # Use NEON (ARM's SIMD extension, 128-bit)
    -mfloat-abi=hard # Hard-float ABI: float args in VFP registers
    -march=armv7-a  # Target ARMv7-A architecture (Cortex-A8, A9, A15...)
)
```

### Key Points

- **Sysroot isolation** — The `CMAKE_FIND_ROOT_PATH_MODE_*` settings prevent CMake from
  accidentally picking up host libraries during cross-compilation. This is the most
  common source of subtle cross-compilation bugs.
- **NEON** — `-mfpu=neon` enables ARM's 128-bit SIMD extension, equivalent in purpose to
  x86 SSE2. Combined with `-mfloat-abi=hard`, the generated code can use NEON for both
  floating-point and vectorized integer operations.
- **ARMv7-A** — Targets the application-class Cortex-A series. If you need to support
  older ARMv6 (Raspberry Pi 1), use `-march=armv6`.

---

## ARM Bare-Metal (none-eabi) Toolchain

**File:** `toolchain/arm-none-eabi.cmake`

This toolchain targets Cortex-M microcontrollers running with no operating system —
firmware, RTOSes, bootloaders.

### Installation

```bash
sudo apt install gcc-arm-none-eabi g++-arm-none-eabi
```

### Full Annotated File

```cmake
# "Generic" tells CMake there is no operating system at all.
# This disables OS-specific checks and prevents CMake from trying to link
# against a standard C runtime startup that requires an OS.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

find_program(CMAKE_C_COMPILER   arm-none-eabi-gcc)
find_program(CMAKE_CXX_COMPILER arm-none-eabi-g++)
find_program(CMAKE_AR           arm-none-eabi-ar)
find_program(CMAKE_OBJCOPY      arm-none-eabi-objcopy) # Convert ELF → binary/hex
find_program(CMAKE_SIZE         arm-none-eabi-size)    # Report code/data/BSS sizes

# Critical for bare-metal: without this, CMake tries to link a test executable
# during the compiler identification phase. That link will fail because there is
# no C runtime startup (crt0.o) available. STATIC_LIBRARY bypasses the link step.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# CPU-specific flags — change these for your hardware target.
# -mcpu=cortex-m4: Cortex-M4 core (ARMv7E-M, DSP instructions)
# -mthumb: Emit Thumb-2 instructions (16/32-bit mixed encoding, smaller code)
# -mfpu=fpv4-sp-d16: FPU variant on Cortex-M4 (single-precision, 16 registers)
# -mfloat-abi=hard: Float args in FPU registers (requires the FPU above)
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT
    "${CPU_FLAGS} -ffunction-sections -fdata-sections")
# -ffunction-sections / -fdata-sections: each function/variable in its own ELF
# section. Combined with --gc-sections in the linker, this dead-strips any code
# or data not reachable from the entry point. Essential for flash-constrained targets.

set(CMAKE_CXX_FLAGS_INIT
    "${CPU_FLAGS} -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti")
# -fno-exceptions: disables C++ exception support. Exception handling tables and
# the unwinder add ~20–50 KB of flash overhead and are generally avoided in
# embedded C++. Throwing across bare-metal would be undefined anyway.
# -fno-rtti: disables runtime type information (dynamic_cast, typeid).
# RTTI tables add non-trivial flash and RAM overhead; rarely useful on MCUs.

set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-Wl,--gc-sections -specs=nosys.specs -specs=nano.specs")
# --gc-sections: garbage-collect unused ELF sections (pairs with -ffunction-sections)
# -specs=nosys.specs: stub out OS syscalls (write, read, sbrk...) so newlib
#   does not require a real OS beneath it.
# -specs=nano.specs: use newlib-nano, a reduced-footprint C library variant.
#   Saves significant flash; trades printf %f support and some POSIX APIs.

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

### Cortex-M4 Specifics

The Cortex-M4 is the most common "advanced" microcontroller core:

- ARMv7E-M architecture (thumb-only execution, DSP extension)
- Optional FPU (fpv4-sp-d16): single-precision only, 16 double-precision or 32 single-
  precision registers. If your target has an FPU and you compile with `-mfloat-abi=soft`,
  you leave performance on the table and break ABI compatibility with hardware-float
  libraries.
- Common in STM32F4xx, NXP LPC43xx, TI Tiva C series.

---

## CMake Presets for Cross-Compilation

From `CMakePresets.json`:

```json
{
  "name": "cross-arm-linux",
  "inherits": "base",
  "displayName": "Cross: ARM Linux",
  "toolchainFile": "${sourceDir}/toolchain/arm-linux-gnueabihf.cmake",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Release"
  }
}
```

```json
{
  "name": "cross-arm-baremetal",
  "inherits": "base",
  "displayName": "Cross: ARM bare-metal",
  "toolchainFile": "${sourceDir}/toolchain/arm-none-eabi.cmake",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "MinSizeRel"
  }
}
```

The bare-metal preset uses `MinSizeRel` (`-Os`) rather than `Release` (`-O2`/`-O3`):
flash memory is typically the binding constraint on MCUs, and code size reduction
matters more than raw throughput.

---

## How to Invoke Each Toolchain

### Method 1: CMake Preset (recommended)

```bash
# ARM Linux
cmake --preset cross-arm-linux
cmake --build --preset cross-arm-linux   # No build preset defined, or add one

# ARM bare-metal
cmake --preset cross-arm-baremetal
cmake --build --preset cross-arm-baremetal
```

### Method 2: Command-Line Toolchain File Flag

When you need to cross-compile a project that does not have presets, or when working
with an existing project:

```bash
# ARM Linux
cmake -B build/arm-linux \
      -DCMAKE_TOOLCHAIN_FILE=toolchain/arm-linux-gnueabihf.cmake \
      -DCMAKE_BUILD_TYPE=Release
cmake --build build/arm-linux

# ARM bare-metal
cmake -B build/arm-baremetal \
      -DCMAKE_TOOLCHAIN_FILE=toolchain/arm-none-eabi.cmake \
      -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build/arm-baremetal
```

### Verifying the Output

After building, confirm you got ARM binaries and not x86:

```bash
file build/arm-linux/pgo_workload
# → ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked

arm-linux-gnueabihf-readelf -h build/arm-linux/pgo_workload | grep Machine
# → Machine: ARM

arm-none-eabi-size build/arm-baremetal/firmware.elf
# →    text    data     bss     dec
# →   12480     408    2048   14936   ...
```

---

## Linker Script Considerations for Bare-Metal

The `arm-none-eabi.cmake` toolchain uses `-specs=nosys.specs` and `-specs=nano.specs`,
which provide a minimal default memory layout. For real embedded targets you must supply
a **linker script** (`.ld` file) that describes your hardware's memory map.

A minimal Cortex-M4 linker script skeleton:

```ld
MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS {
    .text : { *(.isr_vector) *(.text*) *(.rodata*) } > FLASH
    .data : { *(.data*) } > RAM AT > FLASH  /* initialized data: loaded from flash */
    .bss  : { *(.bss*) *(COMMON) } > RAM    /* zero-initialized: not in flash image */
}
```

In CMake, you pass the linker script via:

```cmake
target_link_options(firmware PRIVATE
    "-T${CMAKE_SOURCE_DIR}/linker/stm32f407.ld"
)
```

The linker script ORIGIN addresses must match your MCU's datasheet. Getting them wrong
causes the firmware to execute garbage (FLASH origin wrong) or corrupt stack/heap (RAM
origin wrong).

---

## Interview Talking Points

### Explaining Cross-Compilation Fluency

- **"I know the target triple and what each field means"** — Demonstrate that you can
  decode `arm-linux-gnueabihf` on the spot: ARM architecture, Linux OS, GNU C library,
  hard-float ABI.
- **"I understand sysroot isolation"** — Explain why `CMAKE_FIND_ROOT_PATH_MODE_LIBRARY
  ONLY` is necessary. The interviewer will be impressed if you can describe what goes
  wrong without it: host `libpthread.so` gets linked into an ARM binary, causing linker
  errors or undefined behavior at runtime.
- **"I know the difference between soft-float and hard-float ABI"** — Not just that
  `hf` means "uses the FPU," but that it changes the calling convention: float arguments
  go into `s0–s15` (VFP registers) instead of `r0–r3`. This means a `gnueabi` library
  cannot be linked with a `gnueabihf` binary.

### On Bare-Metal Specifics

- **"`-fno-exceptions` and `-fno-rtti` are nearly always required"** — Explain the flash
  overhead: exception tables and RTTI type strings can add 20–80 KB on a device with
  256 KB of flash. For safety-critical systems, exceptions are often prohibited by the
  coding standard (MISRA, AUTOSAR).
- **"`CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY` is mandatory for bare-metal"** —
  Without it, CMake's compiler identification step tries to link an executable, which
  fails because there is no `crt0.o` or OS entry point. This is a common gotcha when
  first setting up a bare-metal CMake project.
- **"Linker scripts map physical memory"** — Show you understand that `ORIGIN` values
  come from the MCU datasheet, and that `.data` is initialized at startup by copying
  from flash to RAM (the startup code does this before `main()`).

### On Toolchain Portability

- **"A toolchain file makes the build reproducible"** — Any developer with the ARM
  cross-compiler installed can reproduce the build with `cmake --preset cross-arm-linux`.
  No instructions to set `CC=` and `CXX=` manually.
- **"I know when to use the preset and when to pass `-DCMAKE_TOOLCHAIN_FILE`
  directly"** — Presets are convenient for known configurations; the flag form is useful
  when integrating into a CI system that already manages its own build directories.
