#include "pch.h"
#include "PsdMemoryMap.h"
#include "PsdUtils.h"

#define LOG_INSTANCE mLogger

PsdMemoryMap::PsdMemoryMap(PsdLogger *logger)
    : mLogger(logger)
#ifndef _WIN32
    , mFd(-1)
#else
    , mFileHandle(INVALID_HANDLE_VALUE)
    , mMapHandle(NULL)
#endif
{
}

PsdMemoryMap::~PsdMemoryMap()
{
    close();
}

PsdStatus PsdMemoryMap::open(const char *filePath)
{
#ifndef _WIN32
    if (mFd >= 0)
        return PsdStatusFileAlreadyOpened;
    mFd = ::open(filePath, O_RDONLY);
    if (mFd < 0) {
        LOG_ALWAYS("Cannot open %s: %s", filePath, lastErrorMsg());
        return PsdStatusIOError;
    }
    struct stat st;
    if (fstat(mFd, &st)) {
        LOG_ALWAYS("Cannot stat the file: %s", lastErrorMsg());
        return PsdStatusIOError;
    }
    if (!S_ISREG(st.st_mode)) {
        LOG_ALWAYS("%s is not a regular file", filePath, lastErrorMsg());
        return PsdStatusIOError;
    }
    mMapped.mLength = st.st_size;
    mMapped.mAddress = (const uint8_t *) mmap(NULL, mMapped.mLength, PROT_READ, MAP_FILE | MAP_SHARED, mFd, 0);
    if (!mMapped.mAddress) {
        LOG_ALWAYS("%s", lastErrorMsg());
        return PsdStatusMMAP;
    }
#else
    if (mFileHandle != INVALID_HANDLE_VALUE)
        return PsdStatusFileAlreadyOpened;
    mFileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                              FILE_FLAG_RANDOM_ACCESS, NULL);
    if (mFileHandle == INVALID_HANDLE_VALUE) {
        LOG_ALWAYS("Cannot open %s: %s", filePath, lastErrorMsg());
        return PsdStatusIOError;
    }
    LARGE_INTEGER size;
    if (!GetFileSizeEx(mFileHandle, &size)) {
        LOG_ALWAYS("Cannot get the size of the file: %s", lastErrorMsg());
        return PsdStatusIOError;
    }
    if (size.HighPart)
        return PsdStatusFileTooLarge;
    mMapHandle = CreateFileMapping(mFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!mMapHandle) {
        LOG_ALWAYS("CreateFileMapping() failed: %s", lastErrorMsg());
        return PsdStatusMMAP;
    }
    mMapped.mAddress = (const uint8_t *) MapViewOfFile(mMapHandle, FILE_MAP_READ, 0, 0, 0);
    if (!mMapped.mAddress) {
        LOG_ALWAYS("MapViewOfFile() failed: %s", lastErrorMsg());
        return PsdStatusMMAP;
    }
    mMapped.mLength = size.LowPart;
#endif
    return PsdStatusOK;
}

void PsdMemoryMap::close()
{
#ifndef _WIN32
    if (mMapped.mAddress) {
        munmap((void *) mMapped.mAddress, mMapped.mLength);
        mMapped.reset();
    }
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
    }
#else
    if (mMapped.mAddress) {
        UnmapViewOfFile((void *) mMapped.mAddress);
        mMapped.reset();
    }
    if (mMapHandle) {
        CloseHandle(mMapHandle);
        mMapHandle = NULL;
    }
    if (mFileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(mFileHandle);
        mFileHandle = INVALID_HANDLE_VALUE;
    }
#endif
}
