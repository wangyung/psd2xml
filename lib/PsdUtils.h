#pragma once
#ifndef PSDUTILS_H
#define PSDUTILS_H

#include "PsdStatus.h"
#include "PsdDataTypes.h"

namespace PsdUtils
{
    PSDFILE_API const char *getErrorMessage(PsdStatus code);
}

// Display meaningful error message in GTest
#ifdef GTEST_INCLUDE_GTEST_GTEST_H_
inline void PrintTo(const PsdStatus &status, std::ostream *os)
{
    *os << "[(" << (int) status << ") " << PsdUtils::getErrorMessage(status) << "]";
}
#endif

#ifdef LIBPSDFILE_INTERNAL
#include "PsdUtils_int.h"
#endif

#endif // PSDUTILS_H
