#pragma once
#ifndef PSDIO_H
#define PSDIO_H

#include "PsdDataTypes.h"

class PsdIO
{
public:
    // return true on success
    virtual bool write(const uint8_t *buffer, size_t length) = 0;

    virtual int64_t read(uint8_t *buffer, size_t length) = 0;
};

class PsdReader : public PsdIO
{
private:
    virtual bool write(const uint8_t *buffer, size_t length) { return false; };
};

class PsdWriter : public PsdIO
{
private:
    virtual int64_t read(uint8_t *buffer, size_t length) { return -1; };
};

class PsdFileWriter : public PsdWriter
{
public:
    PsdFileWriter();
    ~PsdFileWriter();

    bool open(const char *outputPath);
#ifdef _WIN32
    bool open(const wchar_t *outputPath);
#endif

    virtual bool write(const uint8_t *buffer, size_t length);

private:
    psd_native_io_t mFile;
};

#endif
