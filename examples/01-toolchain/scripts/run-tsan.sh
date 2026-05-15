#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
cmake --preset tsan
cmake --build --preset tsan --target tsan_demo

echo "=== TSan: data race ==="
TSAN_OPTIONS="halt_on_error=1" ./build/tsan/tsan_demo race || true
