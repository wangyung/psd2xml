if(NOT WIN32)
    return()
endif()

add_library(getopt_win32 STATIC
    getopt.c.h
    getopt.h
    getopt.win32.c
    getopt.win32.h
    getopt1.c.h
    getopt_int.h
)
