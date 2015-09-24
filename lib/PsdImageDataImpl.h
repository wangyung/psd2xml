#pragma once
#ifndef PSDIMAGEDATAIMPL_H
#define PSDIMAGEDATAIMPL_H

#include "PsdLayer.h"
#include "PsdImageData.h"
#include <zlib.h>
#include <png.h>

class PsdFile;

class PsdImageDataImpl : public PsdImageData
{
    PSD_OBJECT_CANNOT_COPY(PsdImageDataImpl);

    friend class PsdLayer;
    friend class PsdFile;
    friend class PsdImageData;
public:
    virtual ~PsdImageDataImpl();

    virtual const char *getStoredPNGFilePath() const
    {
        return mFilePath.empty() ? NULL : mFilePath.c_str();
    }
    virtual PsdStatus storePNGFile(const char *outputPath);
    virtual PsdStatus storePNGFile(PsdWriter *writer);
    virtual void setAlpha(double alpha);

private:
    PsdImageDataImpl(PsdLayer *layer);
    PsdImageDataImpl(PsdFile *file);
    PsdImageDataImpl(PsdLayers layers);

    void init();
    PsdStatus readLayerImageData();
    PsdStatus readFileImageData();
    PsdStatus readLayersImageData();
    PsdStatus readLayersImageData(PsdLayers layers);

    PsdStatus writePNGFile(PsdWriter *writer,
                           uint32_t width,
                           uint32_t height,
                           uint32_t x = 0, uint32_t y = 0);

    enum ChannelId
    {
        ChannelIdRealUserSuppliedLayerMask = -3,
        ChannelIdUserSuppliedLayerMask = -2,
        ChannelIdTransparencyMask = -1,
        ChannelIdRed = 0,
        ChannelIdGreen = 1,
        ChannelIdBlue = 2,

        ChannelIdFirst = ChannelIdRealUserSuppliedLayerMask,
        ChannelIdLast = ChannelIdBlue
    };

    enum CompressionMode
    {
        CompressionModeRaw = 0,
        CompressionModeRLE = 1,
        CompressionModeZipNoPredict = 2,
        CompressionModeZipPredict = 3,

        CompressionModeFirst = CompressionModeRaw,
        CompressionModeLast = CompressionModeZipPredict,
        CompressionModeInvalid = CompressionModeLast + 1
    };

    struct ChannelData
    {
        ChannelData()
            : mChannelBuffer(NULL)
            , mCompressionMode(CompressionModeRaw)
        {
        }

        ~ChannelData()
        {
            free((void *) mRawData.mAddress);
        }

        PsdBlock *getChannelBuffer() const
        {
            return mChannelBuffer;
        }

        void releaseBuffer()
        {
            free((void *) mRawData.mAddress);
            mRawData.reset();
            mChannelBuffer = NULL;
        }

        PsdImageDataImpl::CompressionMode mCompressionMode;
        PsdBlock mData;       // might be compressed
        PsdBlock mRawData;    // decompressed
        PsdBlock *mChannelBuffer;
        vector<uint16_t> mRLECounts;
    };

    typedef PsdStatus (PsdImageDataImpl::*StatusCallback)();
    typedef void (PsdImageDataImpl::*ItemCallback)(PsdStatus status, PsdImageDataImpl *imageData);
    PsdStatus forEachImageData(StatusCallback callback, ItemCallback onProcessed = NULL);

    PsdStatus obtainBuffers();
    PsdStatus releaseBuffers();

    void onObtainBuffers(PsdStatus status, PsdImageDataImpl *imageData);

    PsdStatus obtainBuffer(ChannelData *c);
    uint32_t setRLECounts(ChannelData *c, const uint16_t *&rleCounts);
    PsdStatus decodeRLE(uint8_t *dst, const uint8_t *dstEnd, const uint8_t *src, const uint8_t *srcEnd);
    PsdStatus decodeZip(uint8_t *dst, size_t dstLength, const uint8_t *src, size_t srcLength);
    PsdStatus decodeZipPrediction(uint8_t *dst, size_t dstLength, const uint8_t *src, size_t srcLength);

    PsdStatus mergeLayers();
    template<typename T> void addLayer(PsdImageDataImpl *imageData, T **pixels);

    PsdFile *mFile;
    PsdLayer *mLayer;
    PsdLayers mLayers;

    PsdStatus mParseStatus;
    uint32_t mRowBytes; // bytes per row
    uint32_t mWidth;
    uint32_t mHeight;
    shared_ptr<ChannelData> mDataOffseted[ChannelIdLast - ChannelIdFirst + 1];
    shared_ptr<ChannelData> *mData;
    string mFilePath;
    bool mHasAlpha;
    PsdBlock mRawData; // unziped merged image data
    double mAlpha;
    vector<shared_ptr<PsdImageData> > mLayersData;
    union
    {
        uint8_t *mPixels8[4];
        uint16_t *mPixels16[4];
    };

    template<typename T> void writePixelsToPNG(T v,
                           uint32_t width,
                           uint32_t height,
                           uint32_t x = 0, uint32_t y = 0);

    png_structp mPNG;
    png_infop mPNGInfo;
    void pngErrorHandler(png_structp png, png_const_charp msg);
    void pngWarningHandler(png_structp png, png_const_charp msg);
    static void _pngErrorHandler(png_structp png, png_const_charp msg);
    static void _pngWarningHandler(png_structp png, png_const_charp msg);
    static void pngWriteHandler(png_structp png_ptr, png_bytep data, png_size_t length);
};

#endif // PSDIMAGEDATA_H
