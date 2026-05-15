#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
cmake --preset asan
cmake --build --preset asan --target asan_demo

echo "=== ASan: use-after-free ==="
ASAN_OPTIONS="halt_on_error=1:print_stacktrace=1" ./build/asan/asan_demo uaf || true

echo ""
echo "=== ASan: memory leak ==="
ASAN_OPTIONS="halt_on_error=0:detect_leaks=1" LSAN_OPTIONS="print_suppressions=0" \
  ./build/asan/asan_demo leak || true
