#pragma once
#ifndef PSDIMAGEDATA_H
#define PSDIMAGEDATA_H

#include "PsdLayers.h"

class PsdWriter;

class PSDFILE_API PsdImageData
{
    PSD_OBJECT_CANNOT_COPY(PsdImageData);

public:
    virtual const char *getStoredPNGFilePath() const = 0;
    virtual PsdStatus storePNGFile(const char *outputPath) = 0;
    virtual PsdStatus storePNGFile(PsdWriter *writer) = 0;
    virtual void setAlpha(double alpha) = 0;

    virtual ~PsdImageData();

    static shared_ptr<PsdImageData> fromLayers(PsdLayers layers);

protected:
    PsdImageData();
};

#endif // PSDIMAGEDATA_H
