#pragma once
#ifndef PSDLAYERS_H
#define PSDLAYERS_H

class PsdLayer;
class PsdLayerGroup;
typedef shared_ptr<vector<shared_ptr<PsdLayer> > > PsdLayers;

class PSDFILE_API PsdLayersWalker
{
    PSD_OBJECT_CANNOT_COPY(PsdLayersWalker);

public:
    static void walk(PsdLayersWalker &walker, PsdLayers layers);

protected:
    // return false to abort the walker.
    virtual bool onEnterGroup(PsdLayerGroup *group) { return true; }
    virtual bool onLeaveGroup(PsdLayerGroup *group) { return true; }
    virtual bool onLayer(PsdLayer *layer) { return true; }
};

#endif
