#pragma once
#ifndef PSDMEMORYMAP_H
#define PSDMEMORYMAP_H

#include "PsdStatus.h"
#include "PsdDataTypes.h"
#include "PsdLogger.h"

class PSDFILE_API PsdMemoryMap
{
    PSD_OBJECT_CANNOT_COPY(PsdMemoryMap);

public:
    PsdMemoryMap(PsdLogger *logger = PsdLogger::getDefaultInstance());
    ~PsdMemoryMap();

    PsdStatus open(const char *filePath);
    void close();

    const PsdBlock &get() const { return mMapped; }

private:
#ifndef _WIN32
    psd_native_io_t mFd;
#else
    psd_native_io_t mFileHandle;
    psd_native_io_t mMapHandle;
#endif
    PsdBlock mMapped;
    PsdLogger *mLogger;
};

#endif
