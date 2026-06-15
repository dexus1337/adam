# CMakeLists.txt for the asterix module

include(../../cmake/gtest.cmake)
enable_testing()


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Grab all source files for the sdk and tests automatically
file( GLOB_RECURSE  module_source_files "source/*cpp"  )
file( GLOB_RECURSE  test_source_files   "test/*cpp"    ) 


# Create the shared library target for the sdk
add_library(adam-module-asterix SHARED ${module_source_files})

# This tells other targets where to find headers when they link to 'adam-module-asterix'
target_include_directories(adam-module-asterix 
    PUBLIC  include/    # Shared with others
    PRIVATE source/     # Internal to this lib
)

target_link_libraries(adam-module-asterix 
    PRIVATE adam-sdk
)

target_compile_definitions(adam-module-asterix
    PRIVATE ADAM_USE_SHARED_SDK
    PRIVATE ADAM_ASTERIX_EXPORTS
)

# Set the output directory for the module shared library
set_target_properties(adam-module-asterix PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/modules/"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/modules/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/modules/"
)


# Create the test application
add_executable(adam-module-asterix-test ${test_source_files})

target_include_directories(adam-module-asterix-test 
    PRIVATE include/    # Test doesnt need to share headers with others
    PRIVATE source/     # Internal to this lib
)

target_link_libraries(adam-module-asterix-test 
    PRIVATE adam-sdk
    PRIVATE adam-module-asterix
)

# Set the output directory for the executable
set_target_properties(adam-module-asterix-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/"
)

# On Windows, copy the required DLLs to the test directory so gtest_discover_tests can run
if(MSVC OR WIN32)
    add_custom_command(TARGET adam-module-asterix-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:adam-module-asterix>
            $<TARGET_FILE:adam-sdk>
            $<TARGET_FILE_DIR:adam-module-asterix-test>
    )
endif()

setup_gtest(adam-module-asterix-test)

# Discover tests for the VS Code Test Explorer
include(GoogleTest)
gtest_discover_tests(adam-module-asterix-test)