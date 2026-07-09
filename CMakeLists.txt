cmake_minimum_required(VERSION 3.20)

project(adam VERSION 1.0.0 LANGUAGES C CXX)

# Global C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_FLAGS MATCHES "ADAM_PLATFORM_LINUX")
    link_libraries(dl pthread rt)
endif()

# Debug/Release Macros
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions(ADAM_BUILD_DEBUG)
else()
    add_compile_definitions(ADAM_BUILD_RELEASE)
endif()

# Global link module output folder
link_directories(${CMAKE_BINARY_DIR}/bin/modules)

# Disable annoying warnings 
if(MSVC)
    add_compile_options(/wd4251) # exporting std lib classes in DLLs when using MSVC
    add_compile_options(/wd4996) # use of std::xxx (deprecated/dangerous functions)
endif()

# Add subdirectories in order of dependency
add_subdirectory(sdk)
add_subdirectory(libraries)
add_subdirectory(modules)
add_subdirectory(applications)
