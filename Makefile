include generic.mk

CXXFLAGS+=-g3
CXXFLAGS+=-Os
CXXFLAGS+=-std=c++14
CXXFLAGS+=-Wall -Wextra -Werror

CXXFLAGS+=-Isrc

ifeq ($(DEBUG),1)
  CXXFLAGS+=-g3
  LDFLAGS+=-g
endif

# Reduce executable size
CXXFLAGS+=-ffunction-sections -fdata-sections
LDFLAGS+=-Wl,-gc-sections

#------------------------------------------------------------------------------
SRCS_CW+=\
  src/app_cw.cpp\
  src/common_boxes.cpp\

SRCS_CW+=src/spec_dummy.cpp

TARGETS+=$(BIN)/cw.exe
$(BIN)/cw.exe: $(SRCS_CW:%=$(BIN)/%.o)

#------------------------------------------------------------------------------
SRCS_BINARIZE+=\
  src/app_binarize.cpp\

TARGETS+=$(BIN)/binarize.exe
$(BIN)/binarize.exe: $(SRCS_BINARIZE:%=$(BIN)/%.o)

#------------------------------------------------------------------------------
everything: $(TARGETS)

