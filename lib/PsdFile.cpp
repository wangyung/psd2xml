#include "pch.h"

#define LOG_INSTANCE mLogger
#include "PsdFile.h"
#include "PsdUtils.h"
#include "PsdImageDataImpl.h"

#define MIN_PSD_SIZE sizeof(PsdFileHeader)

#define LOG_DEBUG_BLOCK(blk) \
    LOG_DEBUG(#blk ": %u + %u", uint32_t((blk).mAddress - mMapped.get().mAddress), (blk).mLength);

PsdFile::PsdFile(const char *filePath, PsdLogger *logger)
    : mFilePath(filePath)
    , mLogger(logger)
    , mMapped(logger)
    , mHasAlpha(false)
    , mIsOpen(false)
{
}

PsdFile::~PsdFile()
{
    close();
}

PsdStatus PsdFile::open()
{
    PsdStatus status;

    status = openMemoryMap();
    if (status != PsdStatusOK)
        return status;

    status = readHeader();
    if (status != PsdStatusOK)
        return status;

    mResourcesBlock.assign(mColorDataBlock.getEndAddress());
    mLayersBlock.assign(mResourcesBlock.getEndAddress());

    status = readLayersInfo();
    if (status != PsdStatusOK)
        return status;

    mImageDataBlock.mAddress = mLayersBlock.getEndAddress();
    mImageDataBlock.mLength = (ptrdiff_t) mMapped.get().getEndAddress() - (ptrdiff_t) mImageDataBlock.mAddress;
    LOG_DEBUG("Image block: %u + %u",
        uint32_t(mImageDataBlock.mAddress - mMapped.get().mAddress), mImageDataBlock.mLength);

    mIsOpen = true;
    return PsdStatusOK;
}

void PsdFile::close()
{
    mIsOpen = false;
    mMapped.close();
    mColorDataBlock.reset();
    mResourcesBlock.reset();
    mLayersBlock.reset();
    mLayersInfoBlock.reset();
    mGlobalLayerMaskBlock.reset();
    mImageDataBlock.reset();
    mFilePath.clear();
    mImageData.reset();
    mLayers.reset();
    mResources.reset();
}

PsdStatus PsdFile::openMemoryMap()
{
    PsdStatus status = mMapped.open(mFilePath.c_str());
    if (status != PsdStatusOK)
        return status;
    if (mMapped.get().mLength < MIN_PSD_SIZE) {
        LOG_ALWAYS("The input file is too small to be a PSD file.");
        mMapped.close();
        return PsdStatusFileMalformed;
    }
    return PsdStatusOK;
}

PsdStatus PsdFile::readHeader()
{
    // read PSD file header
    const PsdFileHeader *header = (const PsdFileHeader *) mMapped.get().mAddress;
    if (!CHECK_FOUR_CHAR(header->mSignature, '8', 'B', 'P', 'S')) {
        LOG_ALWAYS("PSD file signature not found");
        return PsdStatusFileMalformed;
    }
    // All PSD integers are stored in network byte order
    mVersion = ntohs(header->mVersion);
    mChannels = ntohs(header->mChannels);
    mHeight = ntohl(header->mHeight);
    mWidth = ntohl(header->mWidth);
    mDepth = ntohs(header->mDepth);
    mColorMode = (ColorMode) ntohs(header->mColorMode);
    mColorDataBlock.assign(mMapped.get().mAddress + sizeof(PsdFileHeader));

    LOG_DEBUG("version: %u\nchannels: %u\n%d X %d %dbpp\nmode: %d",
              mVersion, mChannels, mWidth, mHeight, mDepth, mColorMode);

    if (mVersion != 1)
        return PsdStatusUnsupportedVersion;

    return PsdStatusOK;
}

PsdStatus PsdFile::readImageResources()
{
    if (!mResourcesBlock.mAddress)
        return PsdStatusNoInit;

    if (mResourcesBlock.mLength == 0)
        return PsdStatusOK;

    const uint8_t *ptr = mResourcesBlock.mAddress, *end = mResourcesBlock.getEndAddress();
    if (end > mMapped.get().getEndAddress())
        return PsdStatusFileOutOfRange;
    mResources.reset(new map<uint16_t, shared_ptr<PsdResource> >);
    uint32_t size;
    uint16_t id;

    end -= 8;
    while (ptr < end && CHECK_FOUR_CHAR(ptr, '8', 'B', 'I', 'M')) {
        ptr += 4;
        id = PsdUtils::fetch16(ptr);
        ptr += 2;
        string name = PsdUtils::fetchPascalString(ptr);
        ptr += PAD2(name.size() + 1);
        size = PsdUtils::fetch32(ptr);
        ptr += 4;
        (*mResources)[id] = shared_ptr<PsdResource>(new PsdResource(id, name, ptr, size));
        ptr += PAD2(size);
        LOG_DEBUG("[RES] %u (%s) %u\n", id, name.c_str(), size);
    }

    return PsdStatusOK;
}

PsdResources PsdFile::getImageResources()
{
    if (!mResources)
        readImageResources();
    return mResources;
}

void PsdFile::readLayerCount(const uint8_t *ptr)
{
    mLayersCount = (int16_t) PsdUtils::fetch16(ptr);
    if (mLayersCount < 0) {
        mHasAlpha = true;
        mLayersCount = -mLayersCount;
    } else
        mHasAlpha = false;
}

PsdStatus PsdFile::readLayersInfo()
{
    if (!mLayersBlock.mAddress)
        return PsdStatusNoInit;

    if (mLayersBlock.mLength == 0)
        return PsdStatusOK;

    if (mLayersBlock.mLength < 6)
        return PsdStatusFileOutOfRange;

    mLayersInfoBlock.assign(mLayersBlock.mAddress);
    LOG_DEBUG_BLOCK(mLayersBlock);
    if (mLayersInfoBlock.mLength == 0) {
        mLayersCount = 0;
        return PsdStatusOK;
    }
    readLayerCount(mLayersInfoBlock.mAddress);
    return PsdStatusOK;
}

PsdStatus PsdFile::readLayers(const uint8_t *&ptr, const uint8_t *end)
{
    if (ptr > mMapped.get().getEndAddress())
        return PsdStatusFileOutOfRange;

    PsdStatus status;
    deque<PsdLayerGroup *> groups;
    PsdLayerGroup *group = NULL;
    PsdLayers allLayers(new vector<shared_ptr<PsdLayer> >());
    allLayers->reserve(mLayersCount);
    mLayers.reset(new vector<shared_ptr<PsdLayer> >);
    mLayers->reserve(mLayersCount / 2 + 1);

    // read the header of each layer and construct the layer group tree
    for (uint16_t i = 0; i < mLayersCount && ptr < end; i++) {
        shared_ptr<PsdLayer> layer(new PsdLayer(this));

        status = layer->readMeta(ptr, end);
        if (status != PsdStatusOK)
            return status;

        allLayers->push_back(layer);

        LOG_DEBUG("layer: %s\n", layer->getName());
        if (layer->getType() == PsdLayer::LayerTypeGroup) {
            if (layer->mIsEndOfGroup) {
                group = new PsdLayerGroup(this);
                groups.push_back(group);
            } else if (group) {
                group->mName = layer->mName;
                groups.pop_back();

                if (groups.empty()) {
                    // No group in the stack, push to the root group
                    mLayers->push_back(shared_ptr<PsdLayer>(group));
                    group = NULL;
                } else {
                    // Otherwise, push to the top group on the stack
                    groups.back()->mLayers->push_back(shared_ptr<PsdLayer>(group));
                    group = groups.back();
                }
            } else {
                LOG_ALWAYS("A group layer is found but no end-of-group layer exists. Ignoring this layer.");
            }
        } else {
            if (group)
                group->mLayers->push_back(layer);
            else
                mLayers->push_back(layer);
        }
        ptr = layer->mAdditionalInfoBlock.getEndAddress();
    }
    if (!groups.empty())
        return PsdStatusLayerGroupNotComplete;

    LOG_DEBUG("Layer image data start: %u\n", uint32_t(ptr - mMapped.get().mAddress));

    // loop over all layers to obtain the address and length of per-layer image data
    vector<shared_ptr<PsdLayer> >::iterator iter = allLayers->begin(), layersEnd = allLayers->end();
    for (; iter != layersEnd; iter++) {
        PsdLayer *layer = iter->get();

        if (ptr + layer->mImageDataBlock.mLength > end)
            return PsdStatusFileOutOfRange;
        LOG_DEBUG("Layer image data: %u\n", uint32_t(ptr - mMapped.get().mAddress));
        layer->mImageDataBlock.mAddress = ptr;
        ptr += layer->mImageDataBlock.mLength;
    }

    LOG_DEBUG("Layer image data end: %u\n", uint32_t(ptr - mMapped.get().mAddress));

    return PsdStatusOK;
}

PsdStatus PsdFile::readLayers()
{
    if (!mIsOpen)
        return PsdStatusNoInit;

    PsdStatus status;
    const uint8_t *ptr, *end = mLayersBlock.getEndAddress();
    if (mLayersCount > 0 && mLayersInfoBlock.mLength) {
        ptr = mLayersBlock.mAddress + 6;
        status = readLayers(ptr, end);
        if (status != PsdStatusOK)
            return status;
    } else
        ptr = mLayersInfoBlock.getEndAddress();

    if (((ptrdiff_t) ptr - (ptrdiff_t) mMapped.get().mAddress) & 1) // global layer mask info is 2 bytes aligned
        ptr++;
    if (ptr + 4 > end)
        return PsdStatusFileOutOfRange;
    mGlobalLayerMaskBlock.assign(ptr);
    LOG_DEBUG_BLOCK(mGlobalLayerMaskBlock);

    mAdditionalInfoBlock.mAddress = mGlobalLayerMaskBlock.getEndAddress();
    mAdditionalInfoBlock.mLength = (ptrdiff_t) mLayersBlock.getEndAddress() - (ptrdiff_t) mAdditionalInfoBlock.mAddress;
    LOG_DEBUG_BLOCK(mAdditionalInfoBlock);

    return readAdditionalInfo();
}

PsdStatus PsdFile::readAdditionalInfo()
{
    assert(mAdditionalInfoBlock.mAddress != NULL);

    if (mAdditionalInfoBlock.mLength == 0)
        return PsdStatusOK;

    const uint8_t *ptr = mAdditionalInfoBlock.mAddress, *end = mAdditionalInfoBlock.getEndAddress() - 4;
    const uint8_t *key;
    uint32_t length;
    PsdStatus status;
    while (ptr < end) {
        if (!CHECK_FOUR_CHAR(ptr, '8', 'B', 'I', 'M')
            && !CHECK_FOUR_CHAR(ptr, '8', 'B', '6', '4')) {
            LOG_ALWAYS("Global addtional info signature is not found. (%u)",
                uint32_t(ptr - mMapped.get().mAddress));
            return PsdStatusOK;
        }

        key = ptr + 4;
        length = PsdUtils::fetch32(ptr + 8);
        ptr += 12;
        if (CHECK_FOUR_CHAR(key, 'L', 'r', '1', '6')) {
            LOG_DEBUG("Layers are stored in global additional info.");
            readLayerCount(ptr);
            if (mLayersCount > 0) {
                const uint8_t *data = ptr + 2;
                status = readLayers(data, ptr + length);
                if (status != PsdStatusOK)
                    return status;
            }
        }

        // XXX test shows it should be pad by 4, psdparse is not able to parse this though
        ptr += PAD4(length);
    }

    return PsdStatusOK;
}

PsdLayers PsdFile::getLayers()
{
    if (!mLayers)
        readLayers();
    return mLayers;
}

shared_ptr<PsdImageData> PsdFile::getImageData()
{
    if (!mImageData && mIsOpen)
        mImageData.reset(new PsdImageDataImpl(this));
    return mImageData;
}
