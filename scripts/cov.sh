#!/usr/bin/env bash
# Helper script to generate a coverage report for the full test suite.
# Usage: [BRANCH_COVERAGE=1] scripts/cov.sh
# The result is at: cov-html/index.html
set -euo pipefail

# Use --rc so lcov forwards the setting to geninfo even when it is disabled.
BRANCH_COV_OPT="branch_coverage=0"
if [[ ${BRANCH_COVERAGE:-0} == 1 ]]; then
  BRANCH_COV_OPT="branch_coverage=1"
fi
LCOV_OPTS=(--rc "$BRANCH_COV_OPT")

if [[ $(gcov --version) == *LLVM* ]]; then
  LCOV_OPTS+=(--ignore-errors inconsistent,format,category)
fi

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
# Exclude system headers during capture, before consistency checks. The
# leading wildcard also matches headers inside Apple's SDK directories.
lcov "${LCOV_OPTS[@]}" --capture -d $BIN \
  --ignore-errors unused \
  --exclude '*/usr/include/*' \
  --exclude '*/usr/lib/*' \
  --exclude '*/extra/*' \
  -o $BIN/profile.txt

genhtml "${LCOV_OPTS[@]}" -o cov-html $BIN/profile.txt

# free disk space
rm -rf $BIN

echo "Coverage report is available in cov-html/index.html"
