#pragma once

#include "compiler_specifics.h"

[[noreturn]] void fatalerror(const char *format, ...) ATTR_PRINTF(1,2);

