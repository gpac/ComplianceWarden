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

SRCS_APP+=\
  src/main.cpp\
  src/common_boxes.cpp\

TARGETS+=$(BIN)/cw.exe
$(BIN)/cw.exe: $(SRCS_APP:%=$(BIN)/%.o)

everything: $(TARGETS)

