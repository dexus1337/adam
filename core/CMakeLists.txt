# CMakeLists.txt for ADAM Core

if(NOT DEFINED ADAM_NO_TESTS)
include(../cmake/gtest.cmake)
enable_testing()
endif()


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Grab all source files for the target and tests automatically
file(GLOB_RECURSE core_source_files    "source/*cpp"  )
file(GLOB_RECURSE test_source_files   "test/*cpp"    ) 


# Create the shared library target
add_library(adam-core SHARED ${core_source_files})

# This tells other targets where to find headers when they link to 'adam-core'
target_include_directories(adam-core 
    PUBLIC  include/    # Shared with others
    PRIVATE source/     # Internal to this lib
)

target_compile_definitions(adam-core
    PRIVATE ADAM_CORE_EXPORTS
)

# Set the output directory for the shared library
set_target_properties(adam-core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    PDB_OUTPUT_DIRECTORY     "${CMAKE_BINARY_DIR}/bin/"
)


if(NOT DEFINED ADAM_NO_TESTS)
# Create the test application
add_executable(adam-core-test ${test_source_files} ${core_source_files})

target_include_directories(adam-core-test 
    PRIVATE include/    # Test doesnt need to share headers with others
)

target_compile_definitions(adam-core-test
    PRIVATE ADAM_CORE_EXPORTS
)

# Set the output directory for the executable
set_target_properties(adam-core-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/"
)

setup_gtest(adam-core-test)
endif()
