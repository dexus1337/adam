# CMakeLists.txt for the cmd application

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Grab all source files for the sdk and tests automatically
file(GLOB_RECURSE adam_cmd_app_source_files       "source/*cpp"   )
file(GLOB_RECURSE adam_cmd_app_test_source_files   "test/*cpp"    ) 

set(app_resources "../../resources/resources.rc")


# Create the shared library target for the sdk
add_executable(adam-cli ${adam_cmd_app_source_files} ${app_resources})

# Link libraries
target_link_libraries(adam-cli PRIVATE adam-sdk)  # Link against the SDK library

# Set the output directory for the executable
set_target_properties(adam-cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


# Create the test application
add_executable(adam-cli-test ${adam_cmd_app_test_source_files})
target_link_libraries(adam-cli-test PRIVATE adam-sdk)  # Link against the SDK library