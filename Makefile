include generic.mk

CXXFLAGS+=-std=c++14
CXXFLAGS+=-Wall -Wextra -Werror

CXXFLAGS+=-Isrc

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
SRCS_CW+=\
  src/app_cw.cpp\
  src/common_boxes.cpp\
  src/utils.cpp\

SRCS_CW+=src/spec_avif.cpp
SRCS_CW+=src/spec_dummy.cpp
SRCS_CW+=src/spec_isobmff.cpp
SRCS_CW+=src/spec_heif.cpp
SRCS_CW+=src/spec_miaf.cpp
SRCS_CW+=src/spec_miaf_audio.cpp
SRCS_CW+=src/spec_miaf_brands.cpp
SRCS_CW+=src/spec_miaf_derivations.cpp
SRCS_CW+=src/spec_miaf_num_pixels.cpp
SRCS_CW+=src/spec_miaf_profiles.cpp
SRCS_CW+=src/spec_utils.cpp

TARGETS+=$(BIN)/cw.exe
$(BIN)/cw.exe: $(SRCS_CW:%=$(BIN)/%.o)

#------------------------------------------------------------------------------
everything: $(TARGETS)

