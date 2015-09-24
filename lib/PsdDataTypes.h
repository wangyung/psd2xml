#pragma once
#ifndef PSDDATATYPES_H
#define PSDDATATYPES_H

typedef uint32_t psd_size_t;

#ifndef _WIN32
typedef int psd_native_io_t;
#elif defined(LIBPSDFILE_INTERNAL)
typedef HANDLE psd_native_io_t;
#else
typedef void *psd_native_io_t;
#endif

struct PSDFILE_API PsdBlock
{
    PsdBlock()
    {
        reset();
    }

    void reset()
    {
        mLength = 0;
        mAddress = NULL;
    }

    const uint8_t *getEndAddress() const
    {
        return mAddress + mLength;
    }

    void assign(const uint8_t *psdBlock);

    uint32_t mLength;
    const uint8_t *mAddress;
};

#pragma pack(push)
#pragma pack(1)

struct PsdRect
{
    int32_t getWidth() const
    {
        assert(mRight >= mLeft);
        return mRight - mLeft;
    }

    int32_t getHeight() const
    {
        assert(mBottom >= mTop);
        return mBottom - mTop;
    }

    int32_t mTop;
    int32_t mLeft;
    int32_t mBottom;
    int32_t mRight;
};

struct PsdFileHeader
{
    char     mSignature[4];
    uint16_t mVersion;
    char     mReserved[6];
    uint16_t mChannels;
    uint32_t mHeight;
    uint32_t mWidth;
    uint16_t mDepth;
    uint16_t mColorMode;
};

struct PsdChannelInfo
{
    int16_t  mId;
    uint32_t mLength;
};

struct PsdLayerRecordMeta1
{
    PsdRect  mRect;
    uint16_t mChannels;
};

struct PsdLayerRecordMeta2
{
    char    mBlendMode[4];
    uint8_t mOpacity;
    uint8_t mClipping;
    uint8_t mFlags;
    uint8_t mFiller;
};

struct PsdMaskInfo
{
    PsdRect   mRect;
    uint8_t   mDefaultColor;
    uint8_t   mFlags;
    uint8_t   mRealFlags;
    uint8_t   mRealDefaultColor;
    PsdRect   mRealRect;
};

#pragma pack(pop)

#endif
