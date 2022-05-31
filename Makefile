include generic.mk

CXXFLAGS+=-std=c++14
CXXFLAGS+=-Wall -Wextra -Werror

CXXFLAGS+=-Isrc/utils -I$(BIN)

ifeq ($(DEBUG),1)
  CXXFLAGS+=-g3
  LDFLAGS+=-g
else
  CXXFLAGS+=-Os
endif

# Reduce executable size
CXXFLAGS+=-ffunction-sections -fdata-sections
LDFLAGS+=-Wl,-gc-sections

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
  src/utils/common_boxes.cpp\
  src/utils/tools.cpp\
  src/utils/av1_utils.cpp\
  src/utils/isobmff_utils.cpp\
  src/utils/isobmff_derivations.cpp\
  src/utils/spec_utils.cpp\

SRCS_CW+=src/specs/av1_hdr10plus/av1_hdr10plus.cpp
SRCS_CW+=src/specs/avif/avif.cpp
SRCS_CW+=src/specs/avif/profiles.cpp
SRCS_CW+=src/specs/avif/utils.cpp
SRCS_CW+=src/specs/isobmff/isobmff.cpp
SRCS_CW+=src/specs/heif/heif.cpp
SRCS_CW+=src/specs/miaf/miaf.cpp
SRCS_CW+=src/specs/miaf/audio.cpp
SRCS_CW+=src/specs/miaf/brands.cpp
SRCS_CW+=src/specs/miaf/derivations.cpp
SRCS_CW+=src/specs/miaf/colours.cpp
SRCS_CW+=src/specs/miaf/num_pixels.cpp
SRCS_CW+=src/specs/miaf/profiles.cpp

SRCS_CW+=$(BIN)/cw_version.cpp

TARGETS+=$(BIN)/cw.exe
$(BIN)/cw.exe: $(SRCS_CW:%=$(BIN)/%.o)

#------------------------------------------------------------------------------
everything: version $(TARGETS)

