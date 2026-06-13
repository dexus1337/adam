# Function: setup_gtest
# Parameters:
#   TARGET_NAME - The name of the executable to setup

function(setup_gtest TARGET_NAME)
    if(NOT DEFINED ENV{GTEST_ROOT} OR "$ENV{GTEST_ROOT}" STREQUAL "")
        message(FATAL_ERROR "GTEST_ROOT environment variable is not set. Please set it to the path of Google Test.")
    endif()

    target_include_directories(${TARGET_NAME} 
        PRIVATE $ENV{GTEST_ROOT}/googletest/include
    )

    target_link_directories(${TARGET_NAME} 
        PRIVATE $ENV{GTEST_ROOT}/lib
    )

    target_link_libraries(${TARGET_NAME} 
        PRIVATE gtest
        PRIVATE gtest_main
    )

    #gtest requires pthreads on unix systems, so we need to link against it
    if(UNIX)
        target_link_libraries(${TARGET_NAME} 
            PRIVATE  pthread
        )
    endif(UNIX)

    # Automatically copy gtest DLLs to the output directory
    if(MSVC)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$ENV{GTEST_ROOT}/lib/gtest.dll"
                "$ENV{GTEST_ROOT}/lib/gtest_main.dll"
                $<TARGET_FILE_DIR:${TARGET_NAME}>
        )
    endif()

    

endfunction()