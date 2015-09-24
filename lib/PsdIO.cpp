#include "pch.h"
#include "PsdIO.h"

PsdFileWriter::PsdFileWriter()
#ifndef _WIN32
    : mFile(-1)
#else
    : mFile(INVALID_HANDLE_VALUE)
#endif
{
}

PsdFileWriter::~PsdFileWriter()
{
#ifndef _WIN32
    if (mFile >= 0)
        close(mFile);
#else
    if (mFile != INVALID_HANDLE_VALUE)
        CloseHandle(mFile);
#endif
}

bool PsdFileWriter::open(const char *outputPath)
{
#ifndef _WIN32
    return (mFile = ::open(outputPath, O_WRONLY | O_CREAT | O_TRUNC, 00666)) >= 0;
#else
    return (mFile = CreateFileA(outputPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)) != INVALID_HANDLE_VALUE;
#endif
}

#ifdef _WIN32
bool PsdFileWriter::open(const wchar_t *outputPath)
{
    return (mFile = CreateFileW(outputPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)) != INVALID_HANDLE_VALUE;
}
#endif

bool PsdFileWriter::write(const uint8_t *buffer, size_t length)
{
#ifndef _WIN32
    return (::write(mFile, buffer, length) == length);
#else
    DWORD dwWritten;
    return (WriteFile(mFile, (LPCVOID) buffer, length, &dwWritten, NULL) && dwWritten == length);
#endif
}
