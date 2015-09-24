#include "pch.h"

#define LOG_INSTANCE mFile->getLogger()
#include "PsdLayer.h"
#include "PsdFile.h"
#include "PsdUtils.h"
#include "PsdImageDataImpl.h"

PsdLayer::PsdLayer(PsdFile *file)
    : mFile(file)
    , mType(LayerTypeNormal)
    , mIsEndOfGroup(false)
{
}

PsdLayer::~PsdLayer()
{
}

PsdStatus PsdLayer::readMeta(const uint8_t *data, const uint8_t *end)
{
    assert(data != NULL);
    LOG_DEBUG("[Layer] %08X\n", uint32_t(data - mFile->mMapped.get().mAddress));

    const uint8_t *dataSanity = data + sizeof(PsdLayerRecordMeta1) + sizeof(PsdLayerRecordMeta2) + 8;
    if (dataSanity > end)
        return PsdStatusFileOutOfRange;

    PsdStatus status;
    const PsdLayerRecordMeta1 *meta1 = (const PsdLayerRecordMeta1 *) data;
    PsdUtils::fetchRect(&mRect, &meta1->mRect);
    // The layer region may exceed the image size
    mAdjustedRect.mRight = mRect.mRight <= mFile->getWidth() ? mRect.mRight : mFile->getWidth();
    mAdjustedRect.mBottom = mRect.mBottom <= mFile->getHeight() ? mRect.mBottom : mFile->getHeight();
    mAdjustedRect.mLeft = mRect.mLeft >= 0 ? (mRect.mLeft <= mAdjustedRect.mRight ? mRect.mLeft : mAdjustedRect.mRight) : 0;
    mAdjustedRect.mTop = mRect.mTop >= 0 ? (mRect.mTop <= mAdjustedRect.mBottom ? mRect.mTop : mAdjustedRect.mBottom) : 0;
    mChannels = ntohs(meta1->mChannels);
    LOG_DEBUG("(%d, %d, %d, %d) (%d, %d, %d, %d) channels: %u\n",
              mRect.mLeft, mRect.mTop, mRect.mRight, mRect.mBottom,
              mAdjustedRect.mLeft, mAdjustedRect.mTop, mAdjustedRect.mRight, mAdjustedRect.mBottom,
              mChannels);
    data += sizeof(PsdLayerRecordMeta1);

    dataSanity += mChannels * sizeof(PsdChannelInfo);
    if (dataSanity > end)
        return PsdStatusFileOutOfRange;
    const PsdChannelInfo *channelsInfo = (const PsdChannelInfo *) data;
    mChannelsInfo.resize(mChannels);
    for (uint16_t i = 0; i < mChannels; i++) {
        mChannelsInfo[i].mId = ntohs(channelsInfo[i].mId);
        mChannelsInfo[i].mLength = ntohl(channelsInfo[i].mLength);
        mImageDataBlock.mLength += mChannelsInfo[i].mLength;
        LOG_DEBUG("channel %u: id: %d len: %u\n", i, mChannelsInfo[i].mId, mChannelsInfo[i].mLength);
    }
    LOG_DEBUG("image data length: %u\n", mImageDataBlock.mLength);
    data += sizeof(PsdChannelInfo) * mChannels;

    if (!CHECK_FOUR_CHAR(data, '8', 'B', 'I', 'M')) {
        LOG_ALWAYS("Blend mode signature not found");
        return PsdStatusFileMalformed;
    }
    data += 4;

    const PsdLayerRecordMeta2 *meta2 = (const PsdLayerRecordMeta2 *) data;
    memcpy(mBlendMode, meta2->mBlendMode, sizeof(mBlendMode));
    mOpacity = meta2->mOpacity;
    mClipping = meta2->mClipping;
    mFlags = meta2->mFlags;
    mFiller = meta2->mFiller;
    if (mFiller != 0) {
        LOG_DEBUG("Filler is not zero\n");
        return PsdStatusFileMalformed;
    }
    mExtraDataBlock.assign((const uint8_t *)(meta2 + 1));
    if (mExtraDataBlock.getEndAddress() > end || mExtraDataBlock.mLength < 4)
        return PsdStatusFileOutOfRange;
    mMaskInfoBlock.assign(mExtraDataBlock.mAddress);
    status = readMaskData();
    if (status != PsdStatusOK)
        return status;
    if (mMaskInfoBlock.getEndAddress() + 4 > end)
        return PsdStatusFileOutOfRange;
    mBlendingRangesBlock.assign(mMaskInfoBlock.getEndAddress());
    mName = PsdUtils::fetchPascalString(mBlendingRangesBlock.getEndAddress());

    mAdditionalInfoBlock.mAddress = mBlendingRangesBlock.getEndAddress() + PAD4(mName.size() + 1);
    if (mExtraDataBlock.getEndAddress() < mAdditionalInfoBlock.mAddress)
        return PsdStatusFileOutOfRange;
    mAdditionalInfoBlock.mLength = (ptrdiff_t) mExtraDataBlock.getEndAddress() - (ptrdiff_t) mAdditionalInfoBlock.mAddress;

    LOG_DEBUG("layer addtional info: %u + %u\n",
              uint32_t(mAdditionalInfoBlock.mAddress - mFile->mMapped.get().mAddress),
              mAdditionalInfoBlock.mLength);

    if (mAdditionalInfoBlock.mLength > 0) {
        status = readAdditionalInfo();
        if (status != PsdStatusOK)
            return status;
    }

    return PsdStatusOK;
}

PsdStatus PsdLayer::readAdditionalInfo()
{
    assert(mAdditionalInfoBlock.mAddress != NULL);

    if (mAdditionalInfoBlock.mLength == 0)
        return PsdStatusOK;

    const uint8_t *ptr = mAdditionalInfoBlock.mAddress, *end = mAdditionalInfoBlock.getEndAddress() - 4;
    const uint8_t *key;
    uint32_t length;
    while (ptr < end) {
        if (!CHECK_FOUR_CHAR(ptr, '8', 'B', 'I', 'M')
            && !CHECK_FOUR_CHAR(ptr, '8', 'B', '6', '4')) {
            LOG_ALWAYS("Layer addtional info signature is not found. (%u)",
                uint32_t(ptr - mFile->mMapped.get().mAddress));
            return PsdStatusFileMalformed;
        }

        key = ptr + 4;
        length = PsdUtils::fetch32(ptr + 8);
        ptr += 12;
        if (CHECK_FOUR_CHAR(key, 'l', 's', 'c', 't')) {
            if (length >= 4) {
                switch (PsdUtils::fetch32(ptr)) {
                    case 3:
                        mIsEndOfGroup = true;
                    case 1:
                    case 2:
                        mType = LayerTypeGroup;
                        break;
                    default:
                        break;
                }
            }
        }
        // *Almost* all group layers has the value 'lset' for 'lnsr'.
        // However, there are some groups does not has this attribute and
        // some layers (not groups) has 'lset'.
        // It seems to be better to not handle 'lset' at all and use the above method to parse groups.
//         else if (mType != LayerTypeGroup && CHECK_FOUR_CHAR(key, 'l', 'n', 's', 'r')) {
//             assert(length == 4);
//
//             if (CHECK_FOUR_CHAR(ptr, 'l', 's', 'e', 't'))
//                 mType = LayerTypeGroup;
//             else if (CHECK_FOUR_CHAR(ptr, 'l', 'a', 'y', 'r'))
//                 mType = LayerTypeNormal;
//             else
//                 mType = LayerTypeUndefined;
//         }
        //LOG_DEBUG("  AddtionalInfo: %.4s %u\n", key, length);

        ptr += length;
    }

    return PsdStatusOK;
}

PsdStatus PsdLayer::readMaskData()
{
    if (mMaskInfoBlock.mLength == 0)
        return PsdStatusOK;
    if (mMaskInfoBlock.mLength == 20 || mMaskInfoBlock.mLength == 36) {
        const PsdMaskInfo *info = (const PsdMaskInfo *) mMaskInfoBlock.mAddress;

        mMask.reset(new PsdMaskInfo);
        PsdUtils::fetchRect(&mMask->mRect, &info->mRect);
        mMask->mDefaultColor = info->mDefaultColor;
        mMask->mFlags = info->mFlags;
        if (mMaskInfoBlock.mLength == 36) {
            mMask->mRealFlags = info->mRealFlags;
            mMask->mRealDefaultColor = info->mRealDefaultColor;
            PsdUtils::fetchRect(&mMask->mRealRect, &info->mRealRect);
        }
        return PsdStatusOK;
    }
    LOG_ALWAYS("Layer mask info size is wrong (%u)", mMaskInfoBlock.mLength);
    return PsdStatusFileMalformed;
}

shared_ptr<PsdImageData> PsdLayer::getImageData()
{
    if (!mImageData)
        mImageData.reset(new PsdImageDataImpl(this));
    return mImageData;
}
