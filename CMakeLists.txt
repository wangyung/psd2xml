cmake_minimum_required(VERSION 2.8.4)
project(psdtool)

option(DELAY_IMPORT "Use delay import shared libraries" OFF)
option(LIBPSDFILE_SHARED "Build shared library of libpsdfile" OFF)

function(set_static_link_for_release)
    if(NOT APPLE AND NOT (WIN32 AND DELAY_IMPORT))
        if(CMAKE_COMPILER_IS_GNUCXX)
            set(static_flags "-static -static-libgcc")
        endif()
        set_target_properties(${ARGN} PROPERTIES LINK_FLAGS_RELEASE "${static_flags}")
    endif()
endfunction()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")
    # omit .dylib
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

# we must link the same variant of CRT with GTest
if(MSVC)
    include("${PROJECT_SOURCE_DIR}/3rdparty/gtest/cmake/internal_utils.cmake")
    fix_default_compiler_settings_()
endif()

if(WIN32)
    set(ZLIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/3rdparty/zlib/include")
    set(PNG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/3rdparty/libpng/include")
    if(MINGW AND DELAY_IMPORT)
        set(ZLIB_LIBRARIES delayimp_libzlib)
        set(PNG_LIBRARIES delayimp_libpng15)
    else()
        set(ZLIB_LIBRARIES zlib)
        set(PNG_LIBRARIES libpng15)
    endif()
    link_directories(
        "${PROJECT_SOURCE_DIR}/3rdparty/zlib/lib"
        "${PROJECT_SOURCE_DIR}/3rdparty/libpng/lib"
    )
    #For temp solution for psdhtml installation, need to re-arrange overall structure after complete tools ready.
    install(
        FILES "${PROJECT_SOURCE_DIR}/3rdparty/zlib/lib/zlib1.dll" "${PROJECT_SOURCE_DIR}/3rdparty/libpng/lib/libpng15.dll"
        DESTINATION ./psdhtml/libexec
        COMPONENT psdhtml
    )
    set(CPACK_GENERATOR ZIP)
else()
    find_package(PNG REQUIRED)
    set(CPACK_GENERATOR TGZ)
endif()

include_directories(
    ${ZLIB_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/3rdparty"
    "${PROJECT_SOURCE_DIR}/3rdparty/gtest/include"
)

include(PCH)

enable_testing()
set(CPACK_COMPONENTS_ALL psdhtml)
set(CPACK_PACKAGE_VERSION "0.1")
include(CPack)

if(CMAKE_COMPILER_IS_GNUCC)
    set(cflags "-Wall -Werror=return-type")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cflags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cflags} -Wno-reorder")
endif()

set(PSDFILE_LIBRARIES psdfile ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})
if(WIN32)
    set(PSDFILE_LIBRARIES ${PSDFILE_LIBRARIES} ws2_32 getopt_win32)
endif()

if(LIBPSDFILE_SHARED)
    add_definitions(-DLIBPSDFILE_SHARED)
endif()

add_subdirectory(3rdparty/gtest)

add_subdirectory(lib)
add_subdirectory(getopt)
add_subdirectory(console)
#add_subdirectory(gui)
add_subdirectory(tests)
add_subdirectory(html)
