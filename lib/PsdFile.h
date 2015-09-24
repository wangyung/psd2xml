#pragma once
#ifndef PSDFILE_H
#define PSDFILE_H

#include "PsdLayerGroup.h"
#include "PsdResource.h"
#include "PsdLogger.h"
#include "PsdMemoryMap.h"

class PSDFILE_API PsdFile
{
    PSD_OBJECT_CANNOT_COPY(PsdFile);

    friend class PsdLayer;
    friend class PsdImageDataImpl;
public:
    PsdFile(const char *filePath, PsdLogger *logger = PsdLogger::getDefaultInstance());

    virtual ~PsdFile();

    PsdStatus open();
    void close();

    bool isOpen() const
    {
        return mIsOpen;
    }

    //
    // File Header
    //
    uint16_t getVersion() const
    {
        assert(mIsOpen);
        return mVersion;
    }

    uint16_t getChannels() const
    {
        assert(mIsOpen);
        return mChannels;
    }

    uint16_t getDepth() const
    {
        assert(mIsOpen);
        return mDepth;
    }

    uint32_t getWidth() const
    {
        assert(mIsOpen);
        return mWidth;
    }

    uint32_t getHeight() const
    {
        assert(mIsOpen);
        return mHeight;
    }

    enum ColorMode
    {
        ColorModeBitmap = 0,
        ColorModeGrayscale = 1,
        ColorModeIndexed= 2,
        ColorModeRGB = 3,
        ColorModeCMYK = 4,
        ColorModeMultichannel = 7,
        ColorModeDuotone = 8,
        ColorModeLab = 9
    };
    ColorMode getColorMode() const
    {
        assert(mIsOpen);
        return mColorMode;
    }

    //
    // Color Mode Data
    //

    //
    // Image Resources
    //
    PsdResources getImageResources();

    //
    // Layer and Mask Information
    //
    PsdLayers getLayers();

    //
    // Image Data
    //
    shared_ptr<PsdImageData> getImageData();

    PsdLogger *getLogger()
    {
        return mLogger;
    }

private:
    PsdStatus openMemoryMap();
    PsdStatus readHeader();
    PsdStatus readImageResources();
    PsdStatus readLayersInfo();
    PsdStatus readLayers();
    PsdStatus readLayers(const uint8_t *&ptr, const uint8_t *end);
    PsdStatus readAdditionalInfo();
    void readLayerCount(const uint8_t *ptr);

    string mFilePath;
    PsdMemoryMap mMapped;
    bool mIsOpen;

    uint16_t mVersion;
    uint16_t mChannels;
    uint16_t mDepth;
    uint32_t mWidth;
    uint32_t mHeight;
    ColorMode mColorMode;
    int16_t mLayersCount;
    bool mHasAlpha;

    PsdBlock mColorDataBlock;
    PsdBlock mResourcesBlock;
    PsdBlock mLayersBlock;
    PsdBlock mLayersInfoBlock;
    PsdBlock mGlobalLayerMaskBlock;
    PsdBlock mAdditionalInfoBlock;
    PsdBlock mImageDataBlock;

    PsdResources mResources;
    PsdLayers mLayers;

    PsdLogger *mLogger;

    shared_ptr<PsdImageData> mImageData;

    FRIEND_TEST(PsdFileTest, openMemoryMap);
    FRIEND_TEST(PsdFileTest, readHeader);
};

#endif // PSDFILE_H
