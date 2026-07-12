# CMakeLists.txt for the main application

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Grab all source files for the sdk and tests automatically
file(GLOB_RECURSE adam_app_source_files       "source/*cpp"   )
file(GLOB_RECURSE adam_app_test_source_files   "test/*cpp"    ) 

set(app_resources "../../resources/resources.rc")


# Create the shared library target for the sdk
add_executable(adam ${adam_app_source_files} ${app_resources})

# Link libraries
target_link_libraries(adam PRIVATE adam-sdk)  # Link against the SDK library

# Set the output directory for the executable
set_target_properties(adam PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy the install service script to the bin directory depending on the platform
if(WIN32)
    add_custom_command(TARGET adam POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/scripts/service/windows/install-service.ps1"
        "${CMAKE_BINARY_DIR}/bin/install-service.ps1"
    )
elseif(UNIX)
    add_custom_command(TARGET adam POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/scripts/service/linux/install-service.sh"
        "${CMAKE_BINARY_DIR}/bin/install-service.sh"
    )
endif()


if(NOT DEFINED ADAM_NO_TESTS)
# Create the test application
add_executable(adam-test ${adam_app_test_source_files})
target_link_libraries(adam-test PRIVATE adam-sdk)  # Link against the SDK library
endif()
