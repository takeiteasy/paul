#!/bin/sh
# Run all test binaries in the tests/ directory and report results
set -eu

failed=0
total=0

for t in tests/test_*; do
    if [ ! -x "$t" ]; then
        continue
    fi
    total=$((total+1))
    echo "Running $t"
    ./$t
    rc=$?
    if [ $rc -ne 0 ]; then
        echo "FAIL: $t (exit $rc)"
        failed=$((failed+1))
    else
        echo "PASS: $t"
    fi
done

echo "\nTotal: $total, Failures: $failed"
if [ $failed -ne 0 ]; then
    exit 1
fi
exit 0
