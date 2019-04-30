# ComplianceWarden : an pluggable MP4 compliance checker

![](doc/screenshot.png)

# Dependencies

## Build dependencies

 - GNU Bash
 - GNU g++
 - GNU make

## Code formatter (optional)

Install [uncrustify](https://github.com/uncrustify/uncrustify)

```
$ git clone https://github.com/uncrustify/uncrustify.git
$ cd uncrustify
$ git checkout uncrustify-0.64
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j $(nproc)
$ sudo make install
```

# Building

## Native build

```
$ make
```

## Cross-compiling

Simply override CXX to use your target toolchain.

Example for a Windows 64 bit target:

```
$ export CXX=x86_64-w64-mingw32-g++
$ make
```

# Code architecture

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

# Testing

```
./check
```

# Useful information

## Links

 - ISOBMFF specification: (TODO: add link)
 - MP4 specification: (TODO: add link)
 - MIAF specification: (TODO: add link)
 - HEIF specification: (TODO: add link)
 - CMAF specification: (TODO: add link)

