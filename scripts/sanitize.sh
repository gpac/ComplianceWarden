#!/usr/bin/env bash
# Helper script to run an instrumented version
# of the unit tests.
# Usage: ./scripts/sanitize.sh
set -euo pipefail
readonly scriptDir=$(dirname $(readlink -f $0))

# Build instrumented version
export BIN=bin-san
export DEBUG=1

export CXXFLAGS=-fsanitize=address,undefined
export LDFLAGS=-fsanitize=address,undefined
make -j`nproc`

export ASAN_OPTIONS=halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1
$scriptDir/../tests/run $BIN

