#pragma once

#include "Types.h"
#include <string.h>
#include <stdio.h>

char *make_str(FILE **_stream_);
size_t fget_lines(char *filename);