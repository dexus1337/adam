# CMakeLists.txt for the serial module

if(NOT DEFINED ADAM_NO_TESTS)
include(../../cmake/gtest.cmake)
enable_testing()
endif()


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Grab all source files for the sdk and tests automatically
file(GLOB_RECURSE module_source_files "source/*cpp"  )
file(GLOB_RECURSE test_source_files   "test/*cpp"    ) 


# Create the shared library target for the sdk
add_library(adam-module-serial SHARED ${module_source_files})

# This tells other targets where to find headers when they link to 'adam-module-serial'
target_include_directories(adam-module-serial 
    PUBLIC  include/    # Shared with others
    PRIVATE source/     # Internal to this lib
)

target_link_libraries(adam-module-serial 
    PRIVATE adam-sdk
)

target_compile_definitions(adam-module-serial
    PRIVATE ADAM_USE_SHARED_SDK
    PRIVATE ADAM_SERIAL_EXPORTS
)

# Set the output directory for the module shared library
set_target_properties(adam-module-serial PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/modules/"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/modules/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/modules/"
    PDB_OUTPUT_DIRECTORY     "${CMAKE_BINARY_DIR}/bin/modules"
)


if(NOT DEFINED ADAM_NO_TESTS)
# Create the test application
add_executable(adam-module-serial-test ${test_source_files})

target_include_directories(adam-module-serial-test 
    PRIVATE include/    # Test doesnt need to share headers with others
    PRIVATE source/     # Internal to this lib
)

target_link_libraries(adam-module-serial-test 
    PRIVATE adam-sdk
    PRIVATE adam-module-serial
)

# Set the output directory for the executable
set_target_properties(adam-module-serial-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/"
)

setup_gtest(adam-module-serial-test)

# Discover tests for the VS Code Test Explorer
include(GoogleTest)
gtest_discover_tests(adam-module-serial-test)
endif()
