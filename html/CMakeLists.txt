include_directories("${PROJECT_SOURCE_DIR}/lib")

add_executable(psdhtml
    PsdHtml.cpp
    PsdHtml.h
)
target_link_libraries(psdhtml ${PSDFILE_LIBRARIES} ${getopt_win32})

set_static_link_for_release(psdhtml)

install(
    TARGETS psdhtml
    RUNTIME DESTINATION psdhtml/libexec
    COMPONENT psdhtml
)
install(
    DIRECTORY static
    DESTINATION ./psdhtml
    COMPONENT psdhtml
    PATTERN .* EXCLUDE
)
if(WIN32)
    install(
        FILES psdhtml.cmd
        DESTINATION ./psdhtml
        COMPONENT psdhtml
    )
else()
    install(
        PROGRAMS psdhtml.sh
        DESTINATION ./psdhtml
        COMPONENT psdhtml
    )
endif()
