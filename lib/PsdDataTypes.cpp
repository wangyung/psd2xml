#include "pch.h"
#include "PsdDataTypes.h"
#include "PsdUtils.h"

void PsdBlock::assign(const uint8_t *psdBlock)
{
    mLength = PsdUtils::fetch32(psdBlock);
    mAddress = psdBlock + sizeof(mLength);
}
