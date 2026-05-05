#!/usr/bin/env bash
# Full PGO workflow: baseline → instrument → profile → optimized → compare
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_DIR"

echo "=== Step 1: Baseline release build ==="
cmake --preset release
cmake --build --preset release --target pgo_workload
echo "Baseline timing:"
./build/release/pgo_workload

echo ""
echo "=== Step 2: PGO instrumentation build ==="
cmake --preset pgo-generate
cmake --build --preset pgo-generate --target pgo_workload
echo "Running instrumented binary to collect profiles..."
./build/pgo-generate/pgo_workload
echo "Profile files:"
ls *.profraw 2>/dev/null || ls build/pgo-generate/*.profraw 2>/dev/null || echo "(profiles in current directory)"

echo ""
echo "=== Step 3: PGO optimized build ==="
cmake --preset pgo-use
cmake --build --preset pgo-use --target pgo_workload
echo "PGO-optimized timing:"
./build/pgo-use/pgo_workload
