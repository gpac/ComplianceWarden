#!/bin/bash

echo "Reformatting..."
find src -name "*.cpp" -or -name "*.h" | xargs -L1 clang-format -i
