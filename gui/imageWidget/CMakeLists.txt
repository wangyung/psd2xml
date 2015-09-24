FIND_PACKAGE(Qt4 REQUIRED)
IF(QT_FOUND)
  INCLUDE(${QT_USE_FILE})
ELSE(QT_FOUND)
  MESSAGE(FATAL ERROR "Qt4 not found. Call Freddie for help")
ENDIF(QT_FOUND)

#SET(UI_FORMS treeWidget.ui)
#QT4_WRAP_UI(UI_FORMS_WRAP ${UI_FORMS})
#include_directories(${CMAKE_CURRENT_BINARY_DIR})

SET(MOC_HDRS imageWidget.h)
QT4_WRAP_CPP(HEADERS_MOC ${MOC_HDRS})

add_library(imageWidget STATIC imageWidget.cpp
                           ${HEADERS_MOC})
