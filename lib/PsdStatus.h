#pragma once
#ifndef PSDSTATUS_H
#define PSDSTATUS_H

enum PsdStatus
{
    PsdStatusNotImplemented = -1000,

    PsdStatusZipError = -201,
    PsdStatusZipInit = -200,

    PsdStatusPNGError = -101,
    PsdStatusPNGInit = -100,

    PsdStatusNoMemory = -16,
    PsdStatusUnsupportedVersion = -15,
    PsdStatusUnsupportedDepth = -14,
    PsdStatusRLEOutOfRange = -13,
    PsdStatusUnsupportedCompressionMethod = -12,
    PsdStatusFileOutOfRange = -11,
    PsdStatusFileTooLarge = -10,
    PsdStatusLayerGroupNotComplete = -9,
    PsdStatusInvalidArgument = -8,
    PsdStatusNoInit = -7,
    PsdStatusMMAP = -6,
    PsdStatusFileAlreadyOpened = -5,
    PsdStatusUnsupportedColorMode = -4,
    PsdStatusFileMalformed = -3,
    PsdStatusIOError = -2,
    PsdStatusUnknownError = -1,

    PsdStatusOK = 0,

    PsdStatusEmptyImage = 1,
    PsdStatusLayerIsMask = 2
};

#endif // PSDIMAGEDATA_H
