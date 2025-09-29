#!/bin/sh
# Simple test runner for the paul tests and stubs
# Runs any executable matching tests/test_* and tests/stubs/*.{c.out,cpp.out}

set -eu

failures=0
total=0

run_one() {
    path="$1"
    if [ ! -x "$path" ]; then
        return
    fi
    total=$((total + 1))
    printf "Running %s... " "$path"
    if "$path"; then
        echo "PASS"
    else
        echo "FAIL"
        failures=$((failures + 1))
    fi
}

echo "Discovering tests..."

for t in tests/test_*; do
    run_one "$t"
done

echo
echo "Total: $total, Failures: $failures"

if [ "$failures" -ne 0 ]; then
    exit 1
fi

exit 0
