#!/bin/sh
# Build all C and C++ stub sources in tests/stubs/ and report results
set -eu

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)/.."
STUBS_DIR="$ROOT_DIR/tests/stubs"
OUT_DIR="$STUBS_DIR/build"

mkdir -p "$OUT_DIR"

failed=0
total=0

# Choose compilers (allow environment override)
CC=${CC:-cc}
CXX=${CXX:-c++}

echo "Using CC=$CC CXX=$CXX"

for src in "$STUBS_DIR"/*.{c,cpp,cc,cxx}; do
    # If glob didn't match, skip
    if [ ! -e "$src" ]; then
        continue
    fi

    ext="${src##*.}"
    base="$(basename "$src")"
    out="$OUT_DIR/${base%.*}.exe"

    total=$((total+1))
    echo "Compiling $base -> $(basename "$out")"

    rc=0
    if [ "$ext" = "c" ]; then
        "$CC" -std=c11 -Wall -Wextra -Werror -I"$ROOT_DIR" -o "$out" "$src" || rc=$?
    else
        # Treat cpp, cc, cxx as C++ files
        "$CXX" -std=c++17 -Wall -Wextra -Werror -I"$ROOT_DIR" -o "$out" "$src" || rc=$?
    fi

    if [ "$rc" -ne 0 ]; then
        echo "FAIL: $base (exit $rc)"
        failed=$((failed+1))
    else
        echo "PASS: $base"
    fi
done

echo "\nTotal: $total, Failures: $failed"
if [ $failed -ne 0 ]; then
    exit 1
fi
exit 0
