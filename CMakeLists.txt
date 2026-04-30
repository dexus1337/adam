cmake_minimum_required(VERSION 3.20)

project(adam VERSION 1.0.0 LANGUAGES CXX)

# Global C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${CMAKE_CXX_FLAGS} MATCHES "ADAM_PLATFORM_LINUX")
    link_libraries(dl pthread rt)
endif()

# Add subdirectories in order of dependency
add_subdirectory(sdk)
add_subdirectory(libraries)
add_subdirectory(modules)
add_subdirectory(applications)
