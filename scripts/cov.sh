#!/usr/bin/env bash
# Helper script to generate a coverage report for the full test suite.
# Usage: ./cov.sh
# The result is at: bin-cov/html/index.html
set -euo pipefail
rm -rf bin-cov
readonly scriptDir=$(dirname $(readlink -f $0))

# Build instrumented version
export BIN=bin-cov
export CXXFLAGS=--coverage
export LDFLAGS=--coverage
make -j`nproc`

$scriptDir/../tests/run $BIN

# Generate coverage report
find $BIN -path "*/unittests/*.gcda" -delete
lcov --capture -d $BIN -o $BIN/profile-full.txt
lcov --remove $BIN/profile-full.txt \
  --ignore-errors unused \
  '/usr/include/*' \
  '/usr/lib/*' \
  '*/extra/*' \
  -o $BIN/profile.txt

genhtml -o cov-html $BIN/profile.txt

# free disk space
rm -rf $BIN

echo "Coverage report is available in cov-html/index.html"
