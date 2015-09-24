function(add_another_gtest name driver)
    add_test(
        NAME ${name}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND ${driver} "--gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/test-reports/" ${ARGN}
    )
endfunction()

if(MSVC)
    set(include_option "/FI")
else()
    set(include_option "-include ")
endif()

function(add_gtest name)
    add_executable(${name} "${name}.cpp")
    target_link_libraries(${name} gtest TestMain ${ARGN} ${PSDFILE_LIBRARIES})
    set_target_properties(${name} PROPERTIES COMPILE_FLAGS "${include_option}\"${CMAKE_CURRENT_SOURCE_DIR}/TestMain.h\"")
    add_another_gtest(${name} ${name})
endfunction()

add_library(TestMain STATIC TestMain.cpp)

add_gtest(PsdFileTest)
add_gtest(PsdFile_HighLevel_Test)
add_gtest(PsdImageDataTest)
add_another_gtest(OutOfCanvas PsdImageDataTest vectors/out_of_canvas.psd)
add_another_gtest(RGB16_Lr16_1 PsdImageDataTest vectors/automotive_music_library_ui_hd_cs4.psd)
add_another_gtest(RGB16_Lr16_2 PsdImageDataTest vectors/automotive_music_list_expanded_ui_hd.psd)
add_another_gtest(LsetIsNotGroup PsdImageDataTest vectors/set_up_l_dropbox_ui.psd)
