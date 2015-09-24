if(NOT CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
    message(WARNING "Detect cross compilation! GUI will not be built")
    return()
endif()

PROJECT(psd2xml)
add_subdirectory(treeWidget)
add_subdirectory(imageWidget)
find_package(Qt4 REQUIRED)
INCLUDE (${QT_USE_FILE})

SET(psd2xml_SOURCES main.cpp
                    mainwindow.cpp
                    psdFileHdl.cpp)


SET(psd2xml_HEADERS mainwindow.h)
QT4_WRAP_CPP(psd2xml_HEADERS_MOC ${psd2xml_HEADERS})


SET(psd2xml_FORMS mainwindow.ui)
QT4_WRAP_UI(psd2xml_FORMS_HEADERS ${psd2xml_FORMS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

SET(psd2xml_RCCS mainwindow.qrc)
QT4_ADD_RESOURCES(psd2xml_RCC_SRCS ${psd2xml_RCCS})


# build sources, moc'd sources, and rcc'd sources
ADD_EXECUTABLE(psd2xml ${psd2xml_SOURCES}
                       ${psd2xml_HEADERS_MOC}
                       ${psd2xml_FORMS_HEADERS}
                       ${psd2xml_RCC_SRCS})

TARGET_LINK_LIBRARIES(psd2xml treeWidget imageWidget ${QT_LIBRARIES} ${QT_QTGUI_LIBRARY} ${PSDFILE_LIBRARIES})
