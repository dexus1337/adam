# CMakeLists.txt for the SDK

include(../cmake/gtest.cmake)
enable_testing()

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


# Create the test application
add_executable(adam-sdk-test ${test_source_files})

target_include_directories(adam-sdk-test 
    PRIVATE include/    # Test doesnt need to share headers with others
    PRIVATE source/     # Internal to this lib
)

target_link_libraries(adam-sdk-test 
    PRIVATE adam-sdk
)

setup_gtest(adam-sdk-test)

# Discover tests for the VS Code Test Explorer
include(GoogleTest)
gtest_discover_tests(adam-sdk-test)