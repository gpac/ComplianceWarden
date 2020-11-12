#!/usr/bin/env bash
set -euo pipefail

readonly TAG=`git describe --tags --abbrev=0 2> /dev/null || echo "UNKNOWN"`
readonly VERSION=`(git describe --tags --long 2> /dev/null || echo "UNKNOWN") | sed "s/^$TAG-//"`
readonly BRANCH=`git rev-parse --abbrev-ref HEAD 2> /dev/null || echo "UNKNOWN"`
echo "const char* g_version = \"$TAG-$BRANCH-rev$VERSION\";"

