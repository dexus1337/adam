# Function: setup_gtest
# Parameters:
#   TARGET_NAME - The name of the executable to setup

function(setup_gtest TARGET_NAME)

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
    

endfunction()