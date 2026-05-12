# CMakeLists.txt for the SDK

include(../cmake/gtest.cmake)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Grab all source files for the sdk and tests automatically
file( GLOB_RECURSE  sdk_source_files    "source/*cpp"  )
file( GLOB_RECURSE  test_source_files   "test/*cpp"    ) 


# Create the shared library target for the sdk
add_library(adam-sdk SHARED ${sdk_source_files})

# This tells other targets where to find headers when they link to 'adam-sdk'
target_include_directories(adam-sdk 
    PUBLIC  include/    # Shared with others
    PRIVATE source/     # Internal to this lib
)

target_compile_definitions(adam-sdk
    PRIVATE ADAM_SDK_EXPORTS
)

# Set the output directory for the shared library
set_target_properties(adam-sdk PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
)

# Create the test application
add_executable(adam-sdk-test ${test_source_files})

target_include_directories(adam-sdk-test 
    PRIVATE include/    # Test doesnt need to share headers with others
    PRIVATE source/     # Internal to this lib
)

target_link_libraries(adam-sdk-test 
    PRIVATE adam-sdk
)

# Set the output directory for the executable
set_target_properties(adam-sdk-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/"
)

# Automatically copy the required DLLs to the test executable's directory after building
add_custom_command(TARGET adam-sdk-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:adam-sdk>
        $<TARGET_FILE_DIR:adam-sdk-test>
)


setup_gtest(adam-sdk-test)

# Discover tests for the VS Code Test Explorer
include(GoogleTest)
gtest_discover_tests(adam-sdk-test)