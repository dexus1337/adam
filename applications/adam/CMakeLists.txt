# CMakeLists.txt for the main application

# Grab all source files for the sdk and tests automatically
file( GLOB_RECURSE  adam_app_source_files       "source/*cpp"   )
file( GLOB_RECURSE  adam_app_test_source_files   "test/*cpp"    ) 


# Create the shared library target for the sdk
add_executable(adam ${adam_app_source_files})

# Link libraries
target_link_libraries(adam PRIVATE adam-sdk)  # Link against the SDK library


# Create the test application
add_executable(adam-test ${adam_app_test_source_files})
target_link_libraries(adam-test PRIVATE adam-sdk)  # Link against the SDK library