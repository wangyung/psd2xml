#include "pch.h"
#include "PsdLayerGroup.h"

PsdLayerGroup::PsdLayerGroup(PsdFile* file)
    : PsdLayer(file)
    , mLayers(new vector<shared_ptr<PsdLayer> >)
{
    mType = LayerTypeGroup;
    mLayers->reserve(4);
}
