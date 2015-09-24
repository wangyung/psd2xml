#pragma once
#ifndef LIBPSDFILE_PCH_H
#define LIBPSDFILE_PCH_H

#define LIBPSDFILE_INTERNAL

#include "psd_win32.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef _WIN32
    #include <arpa/inet.h>
    #include <sys/mman.h>
#else
    #include <winsock2.h>  // for ntohs() and ntohl()
#endif

#include "psd_common.h"

#endif
