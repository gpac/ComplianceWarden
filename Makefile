BIN?=bin

CXXFLAGS+=-std=c++14
CXXFLAGS+=-Wall -Wextra -Werror

# Reduce executable size
CXXFLAGS+=-fvisibility=hidden -fvisibility-inlines-hidden
CXXFLAGS+=-ffunction-sections -fdata-sections
LDFLAGS+=-Wl,-gc-sections

ifeq ($(DEBUG),1)
  CXXFLAGS+=-g3
  LDFLAGS+=-g
else
  CFLAGS+=-w -DNDEBUG
  LDFLAGS+=-Xlinker -s
  CXXFLAGS+=-O3
endif

CXXFLAGS+=-Isrc/utils -I$(BIN)

all: everything

#------------------------------------------------------------------------------
version:
	@mkdir -p $(BIN)
	@scripts/version.sh > $(BIN)/cw_version.cpp.new
	@if ! diff -q $(BIN)/cw_version.cpp $(BIN)/cw_version.cpp.new >/dev/null ; then \
		mv $(BIN)/cw_version.cpp.new $(BIN)/cw_version.cpp; \
  else \
    rm $(BIN)/cw_version.cpp.new; \
	fi

$(BIN)/cw_version.cpp: version

#------------------------------------------------------------------------------
SRCS_CW+=\
  src/app/cw.cpp\
  src/app/options.cpp\
  src/app/report_std.cpp\
  src/app/report_json.cpp\
  src/utils/common_boxes.cpp\
  src/utils/tools.cpp\
  src/utils/av1_utils.cpp\
  src/utils/isobmff_utils.cpp\
  src/utils/isobmff_derivations.cpp\
  src/utils/spec_utils.cpp\

SRCS_CW+=src/specs/av1_hdr10plus/av1_hdr10plus.cpp
SRCS_CW+=src/specs/avif/avif.cpp src/specs/avif/profiles.cpp src/specs/avif/utils.cpp
SRCS_CW+=src/specs/isobmff/isobmff.cpp
SRCS_CW+=src/specs/heif/heif.cpp
SRCS_CW+=src/specs/uncompressed/uncompressed.cpp
SRCS_CW+=src/specs/miaf/miaf.cpp src/specs/miaf/audio.cpp src/specs/miaf/brands.cpp\
  src/specs/miaf/derivations.cpp src/specs/miaf/colours.cpp src/specs/miaf/num_pixels.cpp\
  src/specs/miaf/profiles.cpp

SRCS_CW+=$(BIN)/cw_version.cpp

#------------------------------------------------------------------------------

TARGETS+=$(BIN)/cw.exe

everything: version $(TARGETS)

$(BIN)/cw.exe: $(SRCS_CW:%=$(BIN)/%.o)

$(BIN)/%.exe:
	@mkdir -p $(dir $@)
	$(CXX) -o "$@" $^ $(LDFLAGS)

$(BIN)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o "$@" "$<"
	@$(CXX) $(CXXFLAGS) -c -o "$@.deps" "$<" -MP -MM -MT "$@"
	@$(CXX) $(CXXFLAGS) -c "$<" -E | wc -l > "$@.lines" # keep track of the code mass

clean:
	rm -rf $(BIN)

-include $(shell test -d $(BIN) && find $(BIN) -name "*.deps")
