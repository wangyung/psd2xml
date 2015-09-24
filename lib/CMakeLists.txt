if(LIBPSDFILE_SHARED)
    set(libtype SHARED)
else()
    set(libtype STATIC)
endif()
add_library(psdfile ${libtype}
    PsdDataTypes.cpp
    PsdDataTypes.h
    PsdFile.cpp
    PsdFile.h
    PsdImageData.h
    PsdImageDataImpl.cpp
    PsdImageDataImpl.h
    PsdIO.cpp
    PsdIO.h
    PsdLayer.cpp
    PsdLayer.h
    PsdLayerGroup.cpp
    PsdLayerGroup.h
    PsdLayers.h
    PsdLayers.cpp
    PsdLayoutGenerator.cpp
    PsdLayoutGenerator.h
    PsdLogger.cpp
    PsdLogger.h
    PsdMemoryMap.cpp
    PsdMemoryMap.h
    PsdResource.h
    PsdStatus.h
    PsdUtils.cpp
    PsdUtils.h
    PsdUtils_int.h
    pch.cpp
    pch.h
    psd_common.h
    psd_file.h
    psd_win32.h
)
add_precompiled_header(psdfile pch.h pch.cpp)

#install(
#    TARGETS psdfile
#    RUNTIME DESTINATION bin
#    ARCHIVE DESTINATION lib
#    LIBRARY DESTINATION lib
#)
