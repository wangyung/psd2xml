#pragma once
#ifndef PSD_COMMON_H
#define PSD_COMMON_H

#include <stdint.h>
#include <assert.h>

#include <vector>
#include <deque>
#include <string>
#include <map>

using std::vector;
using std::deque;
using std::string;
using std::map;

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>

using boost::shared_ptr;
using boost::weak_ptr;
using boost::scoped_ptr;

#ifndef FRIEND_TEST
#define FRIEND_TEST(a, b)
#endif

#define PSD_OBJECT_CANNOT_COPY(type) \
    private: \
        type (const type &); \
        type & operator = (const type &);

#if defined(_WIN32) && defined(LIBPSDFILE_SHARED)
    #ifdef LIBPSDFILE_INTERNAL
        #define PSDFILE_API __declspec(dllexport)
    #else
        #define PSDFILE_API __declspec(dllimport)
    #endif
#else
    #define PSDFILE_API
#endif

#endif
