#pragma once
#ifndef PSDLAYER_H
#define PSDLAYER_H

#include "PsdUtils.h"
#include "PsdDataTypes.h"

struct PsdLayerBase : public PsdLayerRecordMeta1, public PsdLayerRecordMeta2
{
    PsdBlock mExtraDataBlock;
    PsdBlock mMaskInfoBlock;
    PsdBlock mBlendingRangesBlock;
    PsdBlock mAdditionalInfoBlock;
    PsdBlock mImageDataBlock;
};

class PsdFile;
class PsdImageData;
class PsdImageDataImpl;

class PSDFILE_API PsdLayer : protected PsdLayerBase
{
    PSD_OBJECT_CANNOT_COPY(PsdLayer);

    friend class PsdFile;
    friend class PsdImageDataImpl;
public:
    virtual ~PsdLayer();

    const char *getName() const
    {
        return mName.c_str();
    }

    enum LayerType
    {
        LayerTypeNormal,
        LayerTypeGroup,
        LayerTypeUndefined
    };
    LayerType getType() const
    {
        return mType;
    }

    // original region defined in PSD file
    const PsdRect &getRect() const
    {
        return mRect;
    }

    // adjusted region. (may be cropped to fulfill PSD canvas)
    const PsdRect &getAdjustedRect() const
    {
        return mAdjustedRect;
    }

    shared_ptr<PsdImageData> getImageData();

    const PsdMaskInfo *getMaskInfo() const
    {
        return mMask.get();
    }

protected:
    PsdLayer(PsdFile *file);

    struct AddtionalInfo
    {
        uint8_t        mKey[4];
        psd_size_t     mLength;
        const uint8_t *mData;
    };

    PsdFile *mFile;
    LayerType mType;
    string mName;
    vector<PsdChannelInfo> mChannelsInfo;
    shared_ptr<PsdImageData> mImageData;
    scoped_ptr<PsdMaskInfo> mMask;
    PsdRect mAdjustedRect;
    bool mIsEndOfGroup;

private:
    PsdStatus readMeta(const uint8_t *data, const uint8_t *end);
    PsdStatus readAdditionalInfo();
    PsdStatus readMaskData();
};

#endif // PSDLAYER_H
