#pragma once
#ifndef PSDLAYOUTGENERATOR_H
#define PSDLAYOUTGENERATOR_H

#include "PsdFile.h"

class PSDFILE_API PsdLayoutGenerator
{
    PSD_OBJECT_CANNOT_COPY(PsdLayoutGenerator)
public:
    PsdLayoutGenerator(PsdFile *psdFile);

    PsdStatus generate(const char *outputDir);

protected:
    virtual PsdStatus onPNGFileStored(const PsdLayer *layer, const char *filePath)
    {
        return PsdStatusOK;
    }
};

#endif // PSDLAYOUTGENERATOR_H
