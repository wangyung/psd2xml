#include "pch.h"
#include "PsdUtils.h"

namespace PsdUtils
{

void fetchRect(PsdRect *dst, const PsdRect *src)
{
    dst->mTop = ntohl(src->mTop);
    dst->mLeft = ntohl(src->mLeft);
    dst->mBottom = ntohl(src->mBottom);
    dst->mRight = ntohl(src->mRight);
}

PSDFILE_API const char *getErrorMessage(PsdStatus code)
{
    switch (code) {
        case PsdStatusNotImplemented:
            return "The feature has not been implemented.";
        case PsdStatusZipError:
            return "Failed to unzip the data.";
        case PsdStatusZipInit:
            return "Failed to initialize zlib.";
        case PsdStatusPNGError:
            return "Failed to do libpng operations.";
        case PsdStatusPNGInit:
            return "Failed to initialize libpng.";
        case PsdStatusNoMemory:
            return "No memory.";
        case PsdStatusUnsupportedVersion:
            return "The file version is not supported. (PSB?)";
        case PsdStatusUnsupportedDepth:
            return "The color depth is not supported.";
        case PsdStatusRLEOutOfRange:
            return "Detect buffer overrun while decoding RLE.";
        case PsdStatusUnsupportedCompressionMethod:
            return "The compression mode is not supported.";
        case PsdStatusFileOutOfRange:
            return "Detect buffer overrun while parsing PSD structure.";
        case PsdStatusFileTooLarge:
            return "Input file is too large (> 4GB).";
        case PsdStatusLayerGroupNotComplete:
            return "At least one layer group is not completed with a close tag.";
        case PsdStatusInvalidArgument:
            return "Invaild argument.";
        case PsdStatusNoInit:
            return "Operation requires other initialization.";
        case PsdStatusMMAP:
            return "Failed to create memory mapping.";
        case PsdStatusFileAlreadyOpened:
            return "The file has been opened already.";
        case PsdStatusUnsupportedColorMode:
            return "The color mode is not supported.";
        case PsdStatusFileMalformed:
            return "The file is not PSD or corrupted.";
        case PsdStatusIOError:
            return "IO opreation failed.";
        case PsdStatusUnknownError:
            return "Unexpected error.";
        case PsdStatusOK:
            return "Success.";
        case PsdStatusEmptyImage:
            return "The image is empty.";
        case PsdStatusLayerIsMask:
            return "The layer is a mask.";
    }

    assert(!"BUG! No such status code");
    return NULL;
}

}
