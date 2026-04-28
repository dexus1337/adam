# CMakeLists.txt for the recrep module

include(../../cmake/gtest.cmake)
enable_testing()


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Grab all source files for the sdk and tests automatically
file( GLOB_RECURSE  module_source_files "source/*cpp"  )
file( GLOB_RECURSE  test_source_files   "test/*cpp"    ) 


# Create the shared library target for the sdk
add_library(adam-module-recrep SHARED ${module_source_files})

# This tells other targets where to find headers when they link to 'adam-module-recrep'
target_include_directories(adam-module-recrep 
    PUBLIC  include/    # Shared with others
    PRIVATE source/     # Internal to this lib
)

target_link_libraries(adam-module-recrep 
    PRIVATE adam-sdk
)

# Set the output directory for the module shared library
set_target_properties(adam-module-recrep PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/modules/"
)


# Create the test application
add_executable(adam-module-recrep-test ${test_source_files})

target_include_directories(adam-module-recrep-test 
    PRIVATE include/    # Test doesnt need to share headers with others
    PRIVATE source/     # Internal to this lib
)

target_link_libraries(adam-module-recrep-test 
    PRIVATE adam-sdk
    PRIVATE adam-module-recrep
)

# Set the output directory for the executable
set_target_properties(adam-module-recrep-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/"
)

setup_gtest(adam-module-recrep-test)

# Discover tests for the VS Code Test Explorer
include(GoogleTest)
gtest_discover_tests(adam-module-recrep-test)