#pragma once
#ifndef PSDHTML_H
#define PSDHTML_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifndef _WIN32
#include <getopt.h>
#else
#include <psd_win32.h>
#endif
#include "getopt/getopt.win32.h"

#include <psd_file.h>

#ifndef _WIN32
#define PATH_SEP '/'
#else
#define PATH_SEP '\\'
#endif

#endif
