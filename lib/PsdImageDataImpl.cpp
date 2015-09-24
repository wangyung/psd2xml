#include "pch.h"

#define LOG_INSTANCE mFile->getLogger()
#include "PsdImageDataImpl.h"
#include "PsdFile.h"
#include "PsdUtils.h"
#include "PsdLayers.h"
#include "PsdIO.h"

#include <exception>

#define OP_RGB(OP) do { OP(ChannelIdRed); OP(ChannelIdGreen); OP(ChannelIdBlue); } while (0)
#define OP_RGBA(OP) do { OP_RGB(OP); OP(ChannelIdTransparencyMask); } while (0)
#define ROUND_TO_INT(t, n) t(n + 0.5)

class LibpngException : public std::exception {};

PsdImageData::PsdImageData()
{
}

PsdImageData::~PsdImageData()
{
}

shared_ptr<PsdImageData> PsdImageData::fromLayers(PsdLayers layers)
{
    if (!layers || layers->empty())
        return shared_ptr<PsdImageData>();
    return shared_ptr<PsdImageData>(new PsdImageDataImpl(layers));
}

PsdImageDataImpl::PsdImageDataImpl(PsdLayer* layer)
    : mLayer(layer)
    , mFile(layer->mFile)
{
    init();
}

PsdImageDataImpl::PsdImageDataImpl(PsdFile* file)
    : mLayer(NULL)
    , mFile(file)
{
    init();
}

PsdImageDataImpl::PsdImageDataImpl(PsdLayers layers)
    : mLayers(layers)
    , mLayer(NULL)
    , mFile(layers->at(0)->mFile)
{
    init();
}

PsdImageDataImpl::~PsdImageDataImpl()
{
    free((void *) mRawData.mAddress);
}

void PsdImageDataImpl::init()
{
    mParseStatus = PsdStatusNoInit;
    mPNG = NULL;
    mPNGInfo = NULL;
    mHasAlpha = false;
    // make mData[ChannelId] pointer to correct channel
    mData = mDataOffseted - ChannelIdFirst;
    mWidth = 0;
    mHeight = 0;
    mRowBytes = 0;
    mAlpha = 1.0;
}

void PsdImageDataImpl::setAlpha(double alpha)
{
    if (alpha >= 0.0 && alpha <= 1.0)
        mAlpha = alpha;
    else
        assert(!"alpha value must be 0.0 <= alpha <= 1.0");
}

PsdStatus PsdImageDataImpl::storePNGFile(PsdWriter *writer)
{
    if (!writer)
        return PsdStatusInvalidArgument;

    // TODO support other color modes
    if (mFile->getColorMode() != PsdFile::ColorModeRGB)
        return PsdStatusUnsupportedColorMode;

    if (mFile->getDepth() != 8 && mFile->getDepth() != 16)
        return PsdStatusUnsupportedDepth;

    if (mParseStatus == PsdStatusNoInit) {
        mParseStatus = mLayers ? readLayersImageData() :
            (mLayer ? readLayerImageData() : readFileImageData());
    }
    if (mParseStatus != PsdStatusOK)
        return mParseStatus;

    // mask layer can contains valid image
//    if (mData[ChannelIdRealUserSuppliedLayerMask]
//        || mData[ChannelIdUserSuppliedLayerMask])
//        return PsdStatusLayerIsMask;

    PsdStatus status;

    status = mLayers ? mergeLayers() : obtainBuffers();
    if (status != PsdStatusOK)
        return status;

    status = mLayer ?
                writePNGFile(writer,
                             mLayer->getAdjustedRect().getWidth(),
                             mLayer->getAdjustedRect().getHeight(),
                             mLayer->getAdjustedRect().mLeft - mLayer->getRect().mLeft,
                             mLayer->getAdjustedRect().mTop - mLayer->getRect().mTop) :
                writePNGFile(writer, mWidth, mHeight);

    if (mPNG)
        png_destroy_write_struct(&mPNG, &mPNGInfo);

    releaseBuffers();

    return status;
}

PsdStatus PsdImageDataImpl::storePNGFile(const char *outputPath)
{
    if (!outputPath || !*outputPath)
        return PsdStatusInvalidArgument;

    PsdStatus status;
    {
        PsdFileWriter fileWriter;
        if (!fileWriter.open(outputPath)) {
            LOG_ALWAYS("Failed to open %s for write: %s", outputPath, lastErrorMsg());
            return PsdStatusIOError;
        }
        status = storePNGFile(&fileWriter);
    }
    if (status == PsdStatusOK)
        mFilePath = outputPath;
    else
        unlink(outputPath);

    return status;
}

PsdStatus PsdImageDataImpl::readFileImageData()
{
    if (mFile->mImageDataBlock.mLength <= 2
        || mFile->getWidth() == 0 || mFile->getHeight() == 0)
        return PsdStatusEmptyImage;

    uint16_t channels = mFile->getChannels();

    if (channels > 4)
        return PsdStatusUnsupportedColorMode;

    ChannelId id;
    ChannelData *channelData;
    uint32_t size;
    const uint8_t *ptr = mFile->mImageDataBlock.mAddress;
    uint16_t compression = PsdUtils::fetch16(ptr);
    if (compression > CompressionModeLast)
        return PsdStatusUnsupportedCompressionMethod;
    ptr += 2;

    mWidth = mFile->getWidth();
    mHeight = mFile->getHeight();
    mRowBytes = (mWidth * mFile->getDepth() + 7) / 8;

    #define FOR_EACH_CHANNEL() \
        for (int i = 0, b; i < channels; i++) \
            for (channelData = new ChannelData, \
                id = (mFile->mHasAlpha && i == channels - 1 ? ChannelIdTransparencyMask : (ChannelId) i), \
                mData[id].reset(channelData), \
                channelData->mCompressionMode = (CompressionMode) compression, \
                b = 1; b; b = 0)

    switch (compression) {
        case CompressionModeRaw:
            size = mRowBytes * mHeight;
            FOR_EACH_CHANNEL() {
                channelData->mData.mAddress = ptr;
                channelData->mData.mLength = size;
                ptr += size;
            }
            break;
        case CompressionModeRLE: {
            const uint16_t *rleCounts = (const uint16_t *) ptr;
            ptr += mHeight * channels * sizeof(*rleCounts);
            FOR_EACH_CHANNEL() {
                channelData->mData.mAddress = ptr;
                ptr += setRLECounts(channelData, rleCounts);
            }
            break;
        }
        case CompressionModeZipNoPredict:
        case CompressionModeZipPredict: {
            PsdStatus status;
            mRawData.mLength = mHeight * mRowBytes * channels;
            mRawData.mAddress = (const uint8_t *) malloc(mRawData.mLength);
            if (!mRawData.mAddress)
                return PsdStatusNoMemory;
            status = (this->*(compression == CompressionModeZipNoPredict ? &PsdImageDataImpl::decodeZip : &PsdImageDataImpl::decodeZipPrediction))
                ((uint8_t *) mRawData.mAddress, mRawData.mLength, ptr, (ptrdiff_t) mFile->mImageDataBlock.getEndAddress() - (ptrdiff_t) ptr);
            if (status != PsdStatusOK)
                return status;
            size = mRowBytes * mHeight;
            ptr = mRawData.mAddress;
            FOR_EACH_CHANNEL() {
                // the data has been uncompressed above
                channelData->mCompressionMode = CompressionModeRaw;

                channelData->mData.mAddress = ptr;
                channelData->mData.mLength = size;
                ptr += size;
            }
            break;
        }
    }

    #undef FOR_EACH_CHANNEL

    return PsdStatusOK;
}

PsdStatus PsdImageDataImpl::readLayerImageData()
{
    assert(mLayer != NULL);
    assert(mLayer->mImageDataBlock.mAddress != NULL);

    // Each channel has a 2-bytes data indicates the compression mode
    if (mLayer->getAdjustedRect().getWidth() == 0
        || mLayer->getAdjustedRect().getHeight() == 0
        || mLayer->mImageDataBlock.mLength <= mLayer->mChannels * sizeof(uint16_t))
        return PsdStatusEmptyImage;

    mWidth = mLayer->getRect().getWidth();
    mHeight = mLayer->getRect().getHeight();
    mRowBytes = (mWidth * mFile->getDepth() + 7) / 8;

    ChannelData *channelData;
    const uint8_t *ptr = mLayer->mImageDataBlock.mAddress, *end = mLayer->mImageDataBlock.getEndAddress();
    uint16_t compression;
    for (uint16_t i = 0; i < mLayer->mChannels && ptr + 2 < end; i++) {
        int16_t id = mLayer->mChannelsInfo[i].mId;
        LOG_DEBUG("channel #%d %u", id, uint32_t(ptr - mFile->mMapped.get().mAddress));
        if (id < ChannelIdFirst || id > ChannelIdLast) {
            LOG_ALWAYS("Skip unknown channel #%d", id);
            continue;
        }

        channelData = new ChannelData;
        mData[id].reset(channelData);
        compression = PsdUtils::fetch16(ptr);
        channelData->mCompressionMode = compression > CompressionModeLast ?
            CompressionModeInvalid : (CompressionMode) compression;
        ptr += 2;

        if (channelData->mCompressionMode == CompressionModeRLE) {
            const uint16_t *rleCounts = (const uint16_t *) ptr;
            if (setRLECounts(channelData, rleCounts) > mLayer->mChannelsInfo[i].mLength - 2) {
                LOG_ALWAYS("Sum of RLE counts bigger than the channel image data length");
                return PsdStatusRLEOutOfRange;
            }
            channelData->mData.mAddress = (const uint8_t *) rleCounts;
        } else {
            channelData->mData.mAddress = ptr;
            channelData->mData.mLength = mLayer->mChannelsInfo[i].mLength - 2; // - 2 for CompressionMode
        }

        // The following code can parse mask channel
//         PsdRect rect;
//         switch (id) {
//             case -2: // user supplied layer mask
//                 rect = mLayer->getMaskInfo()->mRect;
//                 break;
//             case -3: // real user supplied layer mask
//                 rect = mLayer->getMaskInfo()->mRealRect;
//                 break;
//             default:
//                 rect = mLayer->getRect();
//                 break;
//         }
//
//         channelData->mWidth = rect.getWidth();
//         channelData->mHeight = rect.getHeight();
//         // # of bytes per row. (from psdparse project)
//         channelData->mRowBytes = (rect.getWidth() * mFile->getDepth() + 7) / 8;

        ptr = channelData->mData.getEndAddress();
    }

    return PsdStatusOK;
}

PsdStatus PsdImageDataImpl::readLayersImageData()
{
    mWidth = mFile->getWidth();
    mHeight = mFile->getHeight();
    mRowBytes = (mWidth * mFile->getDepth() + 7) / 8;
    mHasAlpha = mFile->mHasAlpha;

    assert(mLayersData.empty());
    mLayersData.reserve(32);
    return readLayersImageData(mLayers);
}

PsdStatus PsdImageDataImpl::readLayersImageData(PsdLayers layers)
{
    if (!layers || layers->empty())
        return PsdStatusOK;

    PsdStatus status;
    PsdStatus nonFatalStatus = PsdStatusNoInit;

    vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();
    for (; iter != end; iter++) {
        shared_ptr<PsdLayer> layer = *iter;
        if (layer->getType() == PsdLayer::LayerTypeGroup) {
            status = readLayersImageData(((PsdLayerGroup *) layer.get())->getLayers());
        } else {
            shared_ptr<PsdImageData> imageData = layer->getImageData();
            if (!imageData)
                continue;
            status = ((PsdImageDataImpl *) imageData.get())->readLayerImageData();
            if (status == PsdStatusOK)
                mLayersData.push_back(imageData);
        }
        if (status > PsdStatusOK) {
            if (nonFatalStatus == PsdStatusNoInit)
                nonFatalStatus = status;
            continue;
        }
        if (status != PsdStatusOK)
            return status;
        nonFatalStatus = PsdStatusOK;
    }

    return nonFatalStatus;
}

PsdStatus PsdImageDataImpl::forEachImageData(StatusCallback callback, ItemCallback onProcessed)
{
    PsdStatus status;
    PsdStatus nonFatalStatus = PsdStatusNoInit;

    vector<shared_ptr<PsdImageData> >::iterator iter, end = mLayersData.end();
    for (iter = mLayersData.begin(); iter != end; iter++) {
        status = (((PsdImageDataImpl *) iter->get())->*callback)();
        if (onProcessed)
            (this->*onProcessed)(status, (PsdImageDataImpl *) iter->get());
        if (status > PsdStatusOK) {
            if (nonFatalStatus == PsdStatusNoInit)
                nonFatalStatus = status;
            continue;
        }
        if (status != PsdStatusOK)
            return status;
        nonFatalStatus = PsdStatusOK;
    }

    return nonFatalStatus;
}

PsdStatus PsdImageDataImpl::mergeLayers()
{
    PsdStatus status;

    status = forEachImageData(&PsdImageDataImpl::obtainBuffers, &PsdImageDataImpl::onObtainBuffers);
    if (status != PsdStatusOK)
        return status;

    uint32_t size = mHeight * mRowBytes;
    void *buffer;
    for (int i = mHasAlpha ? ChannelIdTransparencyMask : ChannelIdRed; i <= ChannelIdBlue; i++) {
        ChannelData *channelData = new ChannelData;
        mData[i].reset(channelData);
        channelData->mRawData.mLength = size;
        buffer = malloc(size);
        if (!buffer)
            return PsdStatusNoMemory;
        memset(buffer, 0, size);
        channelData->mRawData.mAddress = (const uint8_t *) buffer;
        channelData->mChannelBuffer = &channelData->mRawData;
        mPixels8[i - ChannelIdTransparencyMask] = (uint8_t *) buffer;
    }

    vector<shared_ptr<PsdImageData> >::reverse_iterator iter, end = mLayersData.rend();
    if (mFile->getDepth() == 8) {
        for (iter = mLayersData.rbegin(); iter != end; iter++)
            addLayer((PsdImageDataImpl *) iter->get(), mPixels8 + 1);
    } else if (mFile->getDepth() == 16) {
        for (iter = mLayersData.rbegin(); iter != end; iter++)
            addLayer((PsdImageDataImpl *) iter->get(), mPixels16 + 1);
    } else
        assert(!"The color depth is not allowed.");

    return PsdStatusOK;
}

PsdStatus PsdImageDataImpl::obtainBuffers()
{
    PsdStatus status;

    for (int i = ChannelIdTransparencyMask; i <= ChannelIdBlue; i++) {
        if (mData[i]) {
            status = obtainBuffer(mData[i].get());
            if (status != PsdStatusOK)
                return status;
            if (i == ChannelIdTransparencyMask)
                mHasAlpha = true;
        } else if (i != ChannelIdTransparencyMask) {
            LOG_ALWAYS("We are using RGB mode but channel #%d is missing", i);
            return PsdStatusUnsupportedColorMode;
        }
    }

    return PsdStatusOK;
}

PsdStatus PsdImageDataImpl::releaseBuffers()
{
    if (mLayers)
        forEachImageData(&PsdImageDataImpl::releaseBuffers);

    // reclaim the buffer for decompressed data
    for (int i = ChannelIdTransparencyMask; i <= ChannelIdBlue; i++) {
        if (mData[i])
            mData[i]->releaseBuffer();
    }

    return PsdStatusOK;
}

void PsdImageDataImpl::onObtainBuffers(PsdStatus status, PsdImageDataImpl *imageData)
{
    if (status == PsdStatusOK && imageData->mHasAlpha)
        mHasAlpha = true;
}

PsdStatus PsdImageDataImpl::obtainBuffer(ChannelData *c)
{
    if (c->mCompressionMode == CompressionModeRaw) {
        c->mChannelBuffer = &c->mData;
        return PsdStatusOK;
    }

    if (c->mCompressionMode == CompressionModeInvalid)
        return PsdStatusUnsupportedCompressionMethod;

    if (!mHeight || !mWidth || !mRowBytes)
        return PsdStatusNoInit;

    PsdStatus status;
    c->mRawData.mLength = mRowBytes * mHeight;
    assert(c->mRawData.mAddress == NULL);
    c->mRawData.mAddress = (const uint8_t *) malloc(c->mRawData.mLength);
    if (!c->mRawData.mAddress)
        return PsdStatusNoMemory;

    c->mChannelBuffer = &c->mRawData;
    switch (c->mCompressionMode) {
        case CompressionModeRLE: {
            const uint8_t *src = c->mData.mAddress, *dstEnd, *srcEnd;
            uint8_t *dst = (uint8_t *) c->mRawData.mAddress;
            for (uint32_t i = 0; i < mHeight; i++) {
                dstEnd = dst + mRowBytes;
                srcEnd = src + c->mRLECounts[i];
                status = decodeRLE(dst, dstEnd, src, srcEnd);
                if (status != PsdStatusOK)
                    return status;
                src = srcEnd;
                dst = (uint8_t *) dstEnd;
            }
            return PsdStatusOK;
        }
        case CompressionModeZipNoPredict:
            return decodeZip((uint8_t *) c->mRawData.mAddress, c->mRawData.mLength, c->mData.mAddress, c->mData.mLength);
        case CompressionModeZipPredict:
            return decodeZipPrediction((uint8_t *) c->mRawData.mAddress, c->mRawData.mLength, c->mData.mAddress, c->mData.mLength);
    }

    assert(!"BUG! Should not be here");
    return PsdStatusUnsupportedCompressionMethod;
}

uint32_t PsdImageDataImpl::setRLECounts(ChannelData *c, const uint16_t *&rleCounts)
{
    uint32_t size = 0;
    uint32_t maxCount = mRowBytes * 2;
    uint16_t rleCount;

    c->mRLECounts.resize(mHeight);
    for (uint32_t h = 0; h < mHeight; h++) {
        rleCount = ntohs(*rleCounts++);
        if (rleCount < 2 || rleCount > maxCount)
            LOG_ALWAYS("Detect incorrect RLE count %u (%04X)", rleCount, rleCount);
        c->mRLECounts[h] = rleCount;
        size += rleCount;
    }

    c->mData.mLength = size;
    return size;
}

// The RLE decoding algorithm (psdparse: unpackbits.c)
PsdStatus PsdImageDataImpl::decodeRLE(uint8_t *dst, const uint8_t *dstEnd, const uint8_t *src, const uint8_t *srcEnd)
{
    const uint8_t *srcBound = srcEnd - 1;
    uint8_t length, c;
    bool srcOverrun = false;

    while (src < srcBound) {
        if (dst >= dstEnd)
            return PsdStatusRLEOutOfRange;

        length = *src++;

        // ignore this byte. (got it from psdparse project) (Why?)
        if (length == 128)
            continue;

        if (length > 128) {
            length = 1 + 256 - length;

            c = *src++;
            if (dst + length > dstEnd) {
                // !! overflow
                LOG_ALWAYS("RLE repeat count overflow: %08X", uint32_t(src - mFile->mMapped.get().mAddress));
                memset(dst, c, (ptrdiff_t) dstEnd - (ptrdiff_t) dst);
                return PsdStatusRLEOutOfRange;
            }

            memset(dst, c, length);
            dst += length;
        } else {
            length++;

            if (src + length > srcEnd) {
                LOG_ALWAYS("RLE source overflow: %08X", uint32_t(src - mFile->mMapped.get().mAddress));
                srcOverrun = true;
                length = srcEnd - src;
            }

            if (dst + length > dstEnd) {
                // !! overflow
                LOG_ALWAYS("RLE copy count overflow: %08X", uint32_t(src - mFile->mMapped.get().mAddress));
                memcpy(dst, src, (ptrdiff_t) dstEnd - (ptrdiff_t) dst);
                return PsdStatusRLEOutOfRange;
            }

            memcpy(dst, src, length);
            dst += length;
            src += length;

            if (srcOverrun)
                return PsdStatusFileOutOfRange;
        }
    }

    return PsdStatusOK;
}

PsdStatus PsdImageDataImpl::decodeZip(uint8_t *dst, size_t dstLength, const uint8_t *src, size_t srcLength)
{
    z_stream stream;
    int state;

    memset(&stream, 0, sizeof(z_stream));
    stream.data_type = Z_BINARY;

    stream.next_in = (Bytef *) src;
    stream.avail_in = srcLength;
    stream.next_out = (Bytef *) dst;
    stream.avail_out = dstLength;

    if (inflateInit(&stream) != Z_OK)
        return PsdStatusZipInit;

    while (stream.avail_out > 0) {
        state = inflate(&stream, Z_PARTIAL_FLUSH);
        if (state == Z_STREAM_END)
            return PsdStatusOK;
        if (state != Z_OK)
            return PsdStatusZipError;
    }

    return PsdStatusOK;
}

PsdStatus PsdImageDataImpl::decodeZipPrediction(uint8_t *dst, size_t dstLength, const uint8_t *src, size_t srcLength)
{
    PsdStatus status = decodeZip(dst, dstLength, src, srcLength);
    if (status != PsdStatusOK)
        return status;

    uint8_t *dstEnd, *rowEnd, *rowBound;

    if (mFile->getDepth() == 8) {
        dstEnd = dst + dstLength - 1;
        for (; dst < dstEnd; dst = rowEnd) {
            rowEnd = dst + mRowBytes;
            rowBound = rowEnd - 1;
            for (; dst < rowBound; dst++)
                dst[1] += dst[0];
        }
    } else if (mFile->getDepth() == 16) {
        dstEnd = dst + dstLength - 3;
        for (; dst < dstEnd; dst = rowEnd) {
            rowEnd = dst + mRowBytes;
            rowBound = rowEnd - 3;
            for (; dst < rowBound; dst += sizeof(uint16_t)) {
                dst[2] += dst[0] + ((dst[1] + dst[3]) >> 8);
                dst[3] += dst[1];
            }
        }
    } else {
        LOG_ALWAYS("The depth is not 8 or 16. Cannot unzip.");
        return PsdStatusZipError;
    }

    return PsdStatusOK;
}

void PsdImageDataImpl::pngErrorHandler(png_structp png, png_const_charp msg)
{
    LOG_ALWAYS("[libpng error] %s", msg);

    png_destroy_write_struct(&mPNG, &mPNGInfo);
    assert(mPNG == NULL);
    assert(mPNGInfo == NULL);

    // libpng expect the error handler does not return
    throw(LibpngException());
}

void PsdImageDataImpl::pngWarningHandler(png_structp png, png_const_charp msg)
{
    LOG_ALWAYS("[libpng warning] %s", msg);
}

void PsdImageDataImpl::_pngErrorHandler(png_structp png, png_const_charp msg)
{
    PsdImageDataImpl *self = (PsdImageDataImpl *) png_get_error_ptr(png);
    self->pngErrorHandler(png, msg);
}

void PsdImageDataImpl::_pngWarningHandler(png_structp png, png_const_charp msg)
{
    PsdImageDataImpl *self = (PsdImageDataImpl *) png_get_error_ptr(png);
    self->pngWarningHandler(png, msg);
}

void PsdImageDataImpl::pngWriteHandler(png_structp png_ptr, png_bytep data, png_size_t length)
{
    if (!((PsdWriter *) png_get_io_ptr(png_ptr))->write(data, length)) {
        PsdImageDataImpl *self = (PsdImageDataImpl *) png_get_error_ptr(png_ptr);
        self->mFile->getLogger()->formatMessage("Failed to write PNG file: %s", lastErrorMsg());
        throw LibpngException();
    }
}

static
void pngFlushHandler(png_structp png_ptr)
{
}

PsdStatus PsdImageDataImpl::writePNGFile(PsdWriter *writer, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    LOG_DEBUG("writePNGFile %u X %u (%d, %d)", width, height, x, y);
    try {
        mPNG = png_create_write_struct(PNG_LIBPNG_VER_STRING, this, _pngErrorHandler, _pngWarningHandler);
        if (!mPNG)
            return PsdStatusPNGInit;

        mPNGInfo = png_create_info_struct(mPNG);
        if (!mPNGInfo)
            return PsdStatusPNGInit;

        // the default IO causes crashes in Windows
        png_set_write_fn(mPNG, (void *) writer, pngWriteHandler, pngFlushHandler);
        png_set_IHDR(mPNG, mPNGInfo, width, height,
                     mFile->getDepth(), (mHasAlpha || mAlpha != 1.0) ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
                     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_write_info(mPNG, mPNGInfo);
        png_set_compression_level(mPNG, Z_BEST_COMPRESSION);

        if (mFile->getDepth() == 8)
            writePixelsToPNG((uint8_t) 0, width, height, x, y);
        else
            writePixelsToPNG((uint16_t) 0, width, height, x, y);
        // pass NULL as the 2nd argument,
        // since we do not write text chunks and the time chunk
        png_write_end(mPNG, NULL);
    } catch (LibpngException e) {
        return PsdStatusPNGError;
    }

    return PsdStatusOK;
}

template<typename T>
void PsdImageDataImpl::writePixelsToPNG(T v, uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    bool alphaAdjusted = mAlpha != 1.0;
    uint16_t channels = mHasAlpha || mAlpha ? 4 : 3;
    vector<T> rowBuffer(width * channels);
    T *begin = rowBuffer.data(), *ptr, alphaValue = PsdUtils::hton(ROUND_TO_INT(T, T(~0) * mAlpha));

    const T *buffer[4];
    buffer[0] = (const T *) mData[ChannelIdRed]->getChannelBuffer()->mAddress;
    buffer[1] = (const T *) mData[ChannelIdGreen]->getChannelBuffer()->mAddress;
    buffer[2] = (const T *) mData[ChannelIdBlue]->getChannelBuffer()->mAddress;
    if (mHasAlpha)
        buffer[3] = (const T *) mData[ChannelIdTransparencyMask]->getChannelBuffer()->mAddress;

    uint32_t offset = mWidth * y, rowBegin;
    for (uint32_t n = 0; n < height; n++) {
        ptr = begin;
        rowBegin = offset;
        offset += x;
        for (uint32_t j = 0; j < width; j++) {
            for (uint16_t i = 0; i < 3; i++)
                *ptr++ = buffer[i][offset];
            // Is this better to loop-unrolling?
            if (mHasAlpha)
                *ptr++ = alphaAdjusted ? PsdUtils::hton(ROUND_TO_INT(T, PsdUtils::ntoh(buffer[3][offset]) * mAlpha)) : buffer[3][offset];
            else if (alphaAdjusted)
                *ptr++ = alphaValue;
            offset++;
        }
        offset = rowBegin + mWidth;

        png_write_row(mPNG, (png_byte *) begin);
    }
}

template<typename T>
void PsdImageDataImpl::addLayer(PsdImageDataImpl *imageData, T **pixels)
{
    const T FULLY_OPAQUE = T(~0), FULLY_TRANSPARENT = 0;
    const double FULLY_OPAQUE_DOUBLE = FULLY_OPAQUE;

    PsdRect rect = imageData->mLayer->getAdjustedRect();

    #define OP_CHANNEL_BUFFER(c) srcPixels[c] = (T *) imageData->mData[c]->getChannelBuffer()->mAddress
    #define OP_USE_SRC(c) pixels[c][offset] = *srcPixels[c]
    #define OP_BLEND(c) pixels[c][offset] = \
            PsdUtils::hton(ROUND_TO_INT(T, \
            (srcA * PsdUtils::ntoh(*srcPixels[c]) + srcDstA * PsdUtils::ntoh(pixels[c][offset])) / a))
    #define OP_NEXT(c) srcPixels[c]++
    #define OP_SKIP(c) srcPixels[c] += _initial_skip
    #define OP_NEXT_ROW(c) srcPixels[c] = nextPixels[c]
    #define OP_MARK_NEXT_ROW(c) nextPixels[c] = srcPixels[c] + _width

    T *_pixels[4], *_nextPixels[4];
    T **srcPixels = _pixels - ChannelIdTransparencyMask, **nextPixels = _nextPixels - ChannelIdTransparencyMask;
    int32_t x, y, _x, _y, _initial_skip, _width = imageData->mWidth;
    size_t offset;

    // handle getAdjustedRect() != getRect() cases
    _x = rect.mLeft - imageData->mLayer->getRect().mLeft;
    _y = rect.mTop - imageData->mLayer->getRect().mTop;
    _initial_skip = _y * _width + _x;

    OP_RGB(OP_CHANNEL_BUFFER);
    if (!mHasAlpha) {
        OP_RGB(OP_SKIP);
        for (y = rect.mTop; y < rect.mBottom; y++) {
            OP_RGB(OP_MARK_NEXT_ROW);
            x = rect.mLeft;
            offset = y * mWidth + x;
            for (; x < rect.mRight; x++) {
                OP_RGB(OP_USE_SRC);
                offset++;
                OP_RGB(OP_NEXT);
            }
            OP_RGB(OP_NEXT_ROW);
        }
    } else if (imageData->mData[ChannelIdTransparencyMask]) {
        OP_CHANNEL_BUFFER(ChannelIdTransparencyMask);
        OP_RGBA(OP_SKIP);

        T src_A, dst_A;
        double a, srcA, dstA, srcDstA;
        for (y = rect.mTop; y < rect.mBottom; y++) {
            OP_RGBA(OP_MARK_NEXT_ROW);
            x = rect.mLeft;
            offset = y * mWidth + x;
            for (; x < rect.mRight; x++) {
                src_A = *srcPixels[ChannelIdTransparencyMask];
                dst_A = pixels[ChannelIdTransparencyMask][offset];
                if (src_A == FULLY_TRANSPARENT) {
                    // no-op
                } else if (src_A == FULLY_OPAQUE || dst_A == FULLY_TRANSPARENT) {
                    // fast-path of the spacial cases
                    OP_RGB(OP_USE_SRC);
                    pixels[ChannelIdTransparencyMask][offset] = src_A;
                } else {
                    src_A = PsdUtils::ntoh(src_A);
                    dst_A = PsdUtils::ntoh(dst_A);
                    srcA = src_A / FULLY_OPAQUE_DOUBLE;
                    dstA = dst_A / FULLY_OPAQUE_DOUBLE;
                    srcDstA = (1.0 - srcA) * dstA;

                    a = srcA + srcDstA;
                    OP_RGB(OP_BLEND);
                    pixels[ChannelIdTransparencyMask][offset] = PsdUtils::hton(ROUND_TO_INT(T, a * FULLY_OPAQUE_DOUBLE));
                }

                offset++;
                OP_RGBA(OP_NEXT);
            }
            OP_RGBA(OP_NEXT_ROW);
        }
    } else {
        OP_RGB(OP_SKIP);
        for (y = rect.mTop; y < rect.mBottom; y++) {
            OP_RGB(OP_MARK_NEXT_ROW);
            x = rect.mLeft;
            offset = y * mWidth + x;
            for (; x < rect.mRight; x++) {
                OP_RGB(OP_USE_SRC);
                pixels[ChannelIdTransparencyMask][offset] = FULLY_OPAQUE;
                offset++;
                OP_RGB(OP_NEXT);
            }
            OP_RGB(OP_NEXT_ROW);
        }
    }
}
