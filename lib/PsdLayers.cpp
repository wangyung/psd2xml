#include "pch.h"
#include "PsdLayers.h"
#include "PsdLayerGroup.h"

void PsdLayersWalker::walk(PsdLayersWalker &walker, PsdLayers layers)
{
    if (!layers || layers->empty())
        return;

    vector<shared_ptr<PsdLayer> >::reverse_iterator iter = layers->rbegin(), end = layers->rend();
    for (; iter != end; iter++) {
        shared_ptr<PsdLayer> layer = *iter;
        if (layer->getType() == PsdLayer::LayerTypeGroup) {
            PsdLayerGroup *group = (PsdLayerGroup *) layer.get();
            if (!walker.onEnterGroup(group))
                return;
            walk(walker, group->getLayers());
            if (!walker.onLeaveGroup(group))
                return;
        } else {
            if (!walker.onLayer(layer.get()))
                return;
        }
    }
}
