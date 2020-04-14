# ComplianceWarden : a pluggable MP4 compliance checker

## Dependencies

### Build dependencies

 - GNU Bash
 - GNU g++ version 7+
 - GNU make

### Code formatter (optional)

Install [uncrustify](https://github.com/uncrustify/uncrustify)

```
$ git clone https://github.com/uncrustify/uncrustify.git
$ cd uncrustify
$ git checkout uncrustify-0.64
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j $(nproc)
$ sudo make install
```

## Building

### Native build

Linux, Windows:
```
$ make
```

MacOS X and BSD-likes:
```
$ CXX=scripts/darwin.sh make
```

### Cross-compiling

Simply override CXX to use your target toolchain.

Example for a Windows 64 bit target:

```
$ export CXX=x86_64-w64-mingw32-g++
$ make
```

### Emscripten (WASM)

```
em++ -std=c++14 src/app_cw.cpp src/utils.cpp src/common_boxes.cpp src/spec_dummy.cpp src/spec_heif.cpp src/spec_miaf.cpp src/spec_miaf_audio.cpp src/spec_miaf_brands.cpp src/spec_miaf_derivations.cpp src/spec_miaf_num_pixels.cpp src/spec_miaf_profiles.cpp -o ComplianceWarden.js -O3 -s WASM=1 -s EXPORTED_FUNCTIONS="['_specFindC', '_specCheckC', '_specListRulesC']" -s FORCE_FILESYSTEM=1 -s EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 --pre-js wasm-fs-pre.js
```

See https://gpac.github.io/ComplianceWarden-wasm/ for a demo.

The HTML integration source code is hosted at https://github.com/gpac/ComplianceWarden-wasm.

## Code architecture

```
check                      Top-level full-test script. Reformats + builds + tests.
                           Must pass without error before each commit.

src/                       Source files

tests/                     Integration tests (tests calling the entry points)
tests/run                  Entry point for the test script. Usage: "tests/run bin"
scripts/cov.sh             Coverage script. Generates a coverage report reflecting the
                           current status of the test suite,
scripts/sanitize.sh        Runs the test suite under asan+ubsan.
```

## Testing

```
./check
```

## Useful information

### Versions

 - ISOBMFF specification: m17277
 - MIAF specification: w18260
 - HEIF specification: w18310

# Acknowledgments

This work was initiated as part of the MPEG MIAF conformance software.

