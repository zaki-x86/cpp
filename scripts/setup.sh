#!/usr/bin/env bash
set -euo pipefail

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

ok()   { echo -e "${GREEN}[OK]${NC}  $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
fail() { echo -e "${RED}[FAIL]${NC} $1"; }

echo "=== C++ Workspace Environment Check ==="
echo ""

# CMake
if cmake --version &>/dev/null; then
  CMAKE_VER=$(cmake --version | head -1 | awk '{print $3}')
  ok "CMake $CMAKE_VER"
else
  fail "CMake not found — install cmake >= 3.25"
fi

# GCC
if g++ --version &>/dev/null; then
  GCC_VER=$(g++ --version | head -1 | awk '{print $NF}')
  ok "GCC $GCC_VER"
else
  warn "GCC not found"
fi

# Clang
if clang++ --version &>/dev/null; then
  CLANG_VER=$(clang++ --version | head -1 | awk '{print $4}')
  ok "Clang $CLANG_VER"
else
  warn "Clang not found"
fi

# Ninja
if ninja --version &>/dev/null; then
  ok "Ninja $(ninja --version)"
else
  warn "Ninja not found — builds will use make (slower)"
fi

# clang-tidy
if clang-tidy --version &>/dev/null; then
  ok "clang-tidy found"
else
  warn "clang-tidy not found — install with: sudo apt install clang-tidy"
fi

# CUDA
if nvcc --version &>/dev/null; then
  CUDA_VER=$(nvcc --version | grep release | awk '{print $6}' | tr -d ',')
  ok "CUDA $CUDA_VER"
else
  warn "nvcc not found — project 04-cuda will be skipped"
fi

# ROS2
if [ -n "${ROS_DISTRO:-}" ]; then
  ok "ROS2 $ROS_DISTRO"
else
  warn "ROS2 not sourced — project 03-ros2 will be skipped"
fi

# Qt6
if pkg-config --exists Qt6Core 2>/dev/null; then
  ok "Qt6 found"
else
  warn "Qt6 not found — project 08-qt will be skipped"
fi

# OpenGL / GLFW
if pkg-config --exists glfw3 2>/dev/null; then
  ok "GLFW3 found"
else
  warn "GLFW3 not found — install: sudo apt install libglfw3-dev"
fi

echo ""
echo "=== Done ==="
