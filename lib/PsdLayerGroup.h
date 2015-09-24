#pragma once
#ifndef PSDLAYERGROUP_H
#define PSDLAYERGROUP_H

#include "PsdLayer.h"
#include "PsdLayers.h"

class PSDFILE_API PsdLayerGroup : public PsdLayer
{
    PSD_OBJECT_CANNOT_COPY(PsdLayerGroup);

    friend class PsdFile;
public:
    PsdLayers getLayers() const
    {
        return mLayers;
    }

private:
    PsdLayerGroup(PsdFile *file);

    PsdLayers mLayers;
};

#endif // PSDLAYERGROUP_H
