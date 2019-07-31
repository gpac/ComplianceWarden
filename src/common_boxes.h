#pragma once

#include "reader.h"

ParseBoxFunc* getParseFunction(uint32_t fourcc);

bool isVisualSampleEntry(uint32_t fourcc);

