# Compliance Warden
# A pluggable compliance checker (ISOBMFF, HEIF/MIAF/AVIF, AV1 HDR10+, AV1-ISOBMFF)

## Introduction

The [Compliance Warden](https://github.com/gpac/ComplianceWarden), often abbreviated as "CW" or "cw" or "the warden", is a compliance checker.
CW has been developed as a reference software for MPEG MIAF, AOM AVIF, and AOM AV1 HDR10+.
It is meant to be extended to check MP4, CMAF, and many other file formats.

CW decouples the processing phases. First it parses the input to build an [AST](https://en.wikipedia.org/wiki/Abstract_syntax_tree) stored in very generic structures. Then it processes the AST to validate sets of rules attached to specifications. This approach offers a lot of flexibility and extensibility.

CW is written in modern C++. Binary test vectors are described in assembly (x86 nasm syntax) because [why](#Test-vectors-edition) or [Why Not](https://twitter.com/daemon404/status/1301885488928878593). CW derives from a more generic effort called [Abstract](https://www.motionspell.com/compliance-testing/) started by contributors from the [GPAC](http://gpac.io) open-source project.

The Compliance Warden is distributed under the [BSD-3 permissive license](https://raw.githubusercontent.com/gpac/ComplianceWarden/master/LICENSE).

## Useful information

### Online version

An online version is available [here](https://gpac.github.io/ComplianceWarden-wasm/). Note that the software is executed in your browser and doesn't upload any data outside your computer.

### Usage

[New option parser](https://github.com/gpac/ComplianceWarden/issues/48) (introduced in July 2023):

```
$ bin/cw.exe -h
Compliance Warden, version v32-master-rev14-g363d8d3

Usage:
    -s, --spec                              Specification name.
    -f, --format                            Output format: "raw" (default), or "json"
    -l, --list                              List available specifications or available rules.
    -v, --version                           Print version and exit.
    -h, --help                              Print usage and exit.
    -t, --test                              Don't print warnings when switching to legacy mode.
```


The old usage is deprecated and will be removed in v34:

```
$ bin/cw.exe
Compliance Warden, version v32-master-rev14-g363d8d3

Usage:
- Run conformance:          bin/cw.exe <spec> input.mp4 [json]
- List specifications:      bin/cw.exe list
- List specification rules: bin/cw.exe <spec> list
- Print version:            bin/cw.exe version
```

### Specifications

CW is mainly [sponsored](#Acknowledgments) by companies and standardization groups to validate specific versions of specifications they develop or use.

The master branch only references official specifications. Draft versions or updates are meant to be in separate branches. To know more please read the [design principles](https://github.com/gpac/ComplianceWarden/issues/55#issuecomment-1517063130).


However, once a specification is validated, we accept to add new rules progressively.

```
$ bin/cw.exe --list
================================================================================
Specification name: av1hdr10plus
            detail: HDR10+ AV1 Metadata Handling Specification, 7 December 2022
https://github.com/AOMediaCodec/av1-hdr10plus/commit/63bacd21bc5f75ea6094fc11a03f0e743366fbdf
https://aomediacodec.github.io/av1-hdr10plus/
        depends on: "av1isobmff" specifications.
================================================================================

================================================================================
Specification name: av1isobmff
            detail: AV1 Codec ISO Media File Format Binding v1.2.0, 12 December 2019
https://github.com/AOMediaCodec/av1-isobmff/commit/ee2f1f0d2c342478206767fb4b79a39870c0827e
https://aomediacodec.github.io/av1-isobmff/v1.2.0.html
        depends on: "isobmff" specifications.
================================================================================

================================================================================
Specification name: avif
            detail: AVIF v1.0.0, 19 February 2019
https://aomediacodec.github.io/av1-avif/
        depends on: "miaf" specifications.
================================================================================

================================================================================
Specification name: isobmff
            detail: ISO Base Media File Format
MPEG-4 part 12 - ISO/IEC 14496-12 - m17277 (6th+FDAM1+FDAM2+COR1-R4)
        depends on: none.
================================================================================

================================================================================
Specification name: heif
            detail: HEIF - ISO/IEC 23008-12 - 2nd Edition N18310
        depends on: "isobmff" specifications.
================================================================================

================================================================================
Specification name: miaf
            detail: MIAF (Multi-Image Application Format)
MPEG-A part 22 - ISO/IEC 23000-22 - w18260 FDIS - Jan 2019
        depends on: "heif" specifications.
================================================================================
```

## Building

### Prerequisites

[NASM](https://nasm.us/) and a C++14 compiler.

### Native build

Linux, Windows:
```
$ make -j
```

MacOS X and BSD-likes:

```
$ CXX=scripts/darwin.sh make -j
```

or

```
$ export CXX=scripts/darwin.sh
$ make -j
```

### Cross-compiling

Simply override CXX to use your target toolchain.

Example for a Windows 64 bit target:

```
$ CXX=x86_64-w64-mingw32-g++ make
```

or

```
$ export CXX=x86_64-w64-mingw32-g++
$ make
```

or

```
$ BIN=bin_32 CXX=i686-linux-gnu-g++-12 ./check
```

### Emscripten (WASM)

```
em++ -std=c++14 -DCW_WASM bin/cw_version.cpp `find src -name '*.cpp'` -Isrc/utils -o ComplianceWarden.js -O3 -s WASM=1 -s EXPORTED_FUNCTIONS="['_specFindC', '_specCheckC', '_specListRulesC', '_printVersion', '_free']" -s FORCE_FILESYSTEM=1 -s EXIT_RUNTIME=0 -s ALLOW_MEMORY_GROWTH=1 --pre-js scripts/wasm-fs-pre.js
```

See https://gpac.github.io/ComplianceWarden-wasm/ for a demo.

The HTML integration source code is hosted at https://github.com/gpac/ComplianceWarden-wasm.

## Testing

The Compliance Warden includes known good tests and known bad tests. This ensures the software is stable to false positives.

```
./check
```

NB: don't forget to set ```CXX``` when your toolchain requires so e.g. for Darwin (MacOS) ```CXX=scripts/darwin.sh ./check```.

## Contributing

### Build dependencies

 - GNU Bash
 - GNU g++ version 7+
 - GNU make
 - NASM 2.01+

### Code formatter (optional)

Install ```clang-format```.

### Pre-commit: format, build and run tests before committing

```
./check
```

### Ensure good code coverage

You need ```lcov```.

```
scripts/cov.sh
```

> Note: On Darwin (MacOS) systems you may need to install GNU version of ```g++``` and ```gcov``` (e.g. ```brew install gcc```). Then change ```./scripts/darwin.sh``` to alias GNU versions instead of Clang versions.

### Modifying test results

The tests (launched with ```./check```) will stop running on first error.

To update the test results, uncomment the ```# cp "$new" "$ref"``` line in the ```tests/run``` script. This avoids tests to halt when an error occurs. Please review carefully the changes before updating test results.

## Code architecture

### Repository file structure

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

### Principles

The Compliance Warden is made of three parts:
 - a file parser ```common_boxes.cpp``` that can be extended (or superseded) by each specification,
 - some array of rules stored in ```specs/```,
 - an application stored in ```src/app/cw.cpp``` that probes the files, launches the tests, and produces a human-readable report.

The parsing is decoupled from the rules. This allows a lot of flexibility such as:
 - the replacement of the parser by an external tool,
 - the implementation of rules in a different language.

The result of the parsing phase is comparable to an [AST](https://en.wikipedia.org/wiki/Abstract_syntax_tree). This AST is then processed by the rules.

The datastructures are generic. This allows to easily serialize them. This is useful when plugging new languages or building new tests.

### Tests

A test is a pair of a file format description in the [NASM syntax](https://en.wikipedia.org/wiki/Netwide_Assembler) ([example](https://raw.githubusercontent.com/gpac/ComplianceWarden/9ebfd86c392221714f42a625673536e43835938c/tests/isobmff/invalid-track-identifiers.asm)) and a reference result ([example](https://raw.githubusercontent.com/gpac/ComplianceWarden/9ebfd86c392221714f42a625673536e43835938c/tests/isobmff/invalid-track-identifiers.ref)).

### Test vectors edition

Test vectors are represented using some x86 assembly, in a textual human-editable form. In practice only labels and two instructions (```db``` to write 8 bits and ```dd``` to write 32 bits) are used. See this example of a ```mdat``` box containing some AV1 OBU:

```
mdat_start:
    dd BE(mdat_end - mdat_start)
    dd "mdat"
     ; obu(0) 
    db 0x0A ; forbidden(1) obu_type(4) obu_extension_flag(1) obu_has_size_field(1) obu_reserved_1bit(1) 
    db 0x0F ; leb128_byte(8) 
mdat_end:
```

At the time of creating the project, we couldn't find any way to create both valid and invalid editable test vectors. Annotated assembly (with symbols' names, num_bits, and values) looked like a sensible choice.

The easiest way is to create a new test vector is to derive an existing one.

When this is not possible, one needs to disassemble an existing binary file. Contact romain.bouqueau@motionspell.com if you need some help. The assembly file needs to be stripped from its data (generally shortining radically the ```mdat``` box) and metadata (removing unused boxes and strings).

The key point is to understand that these test vectors are not intended to be valid media files. We may want to add valid samples to the tests though (e.g. retrieving files and testing them) ; in this case other tools (e.g. ```MP4Box -diso``` or ```gpac -i FILE inspect:deep:analyze=bs```) already provides some deep view of what's in the file.

### Adding a test

A test is a function:
- Input is both a box tree (from the parsing phase) and a link to the report.
- Output is written to the report: warning, errors, and ```covered()``` to assess that the rule was exercised by the input sample.

```
struct RuleDesc
{
  [...]
  // human-readable description of the rule
  const char* caption;

  // optional id from the specification
  const char* id = nullptr;

  // apply this rule to the file 'root',
  // will push the results (messages) to the 'out' report.
  void (* check)(Box const& root, IReport* out);
};
```

### Adding a specification

A specification is just a list of rules with some general information:
```
struct SpecDesc
{
  // short name for the spec (used for command-line spec selection).
  const char* name;

  // human-readable description of the spec (name, version, date, etc.).
  const char* caption;

  // list of specs which this spec depends on.
  std::vector<const char*> dependencies;

  // list of compliance checks for this spec.
  std::vector<RuleDesc> rules;

  // checks will only be executed if this returns true.
  bool (* valid)(Box const& root) = nullptr;
};
```

## Limitations

Some aspects are not activated:
 - Brand presence checks are not fully activated. When activated, relaxed brands (e.g. 'MiPr') emit a lot of messages that bring little value to the user. Aggressive shall/should normative statements need to be balanced at standardization level.
 - Codec-level parsing is incomplete. It should be deferred in most case to an external project that can analyze both the metadata and the data (e.g. [GPAC](http://gpac.io)). This hasn't been done due to licensing concerns.
 - Some rules related to pixel formats (color spaces, ...) (computations and consistency) may only be checked by a player. Hence they are considered outside of the scope of this project.
 - Some rules related to pixel formats are only processed for AV1. Because we embed some codec-level parsing for AV1.
 - Some rules are not implemented due to missing content (e.g. Apple Audio Twos).

## Acknowledgments

This work was initiated as part of the MPEG MIAF conformance software.

The [Alliance for Open Media (AOM)](https://aomedia.org/) sponsored the work on AVIF, AV1-ISOBMFF, and AV1 HDR10+.
