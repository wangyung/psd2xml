#pragma once
#ifndef PSD_WIN32_H
#define PSD_WIN32_H

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0502
#define _WIN32_WINNT 0x0502
#define _WIN32_IE 0x0600
#include <windows.h>

#endif // _WIN32

#endif // PSD_WIN32_H
