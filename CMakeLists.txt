cmake_minimum_required(VERSION 3.20)

project(adam VERSION 1.0.0 LANGUAGES C CXX)

# Global C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(ANDROID)
    link_libraries(dl log)
elseif(CMAKE_CXX_FLAGS MATCHES "ADAM_PLATFORM_LINUX")
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

# Resolve SDL3 dependency (Windows prebuilts or Android source/FetchContent)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/adam-sdl3.cmake")

# Add subdirectories in order of dependency
add_subdirectory(core)
add_subdirectory(libraries)
add_subdirectory(modules)
add_subdirectory(applications)

if(ANDROID)
    set(ANDROID_JNILIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main/jniLibs/${ANDROID_ABI}")
    add_custom_target(stage-android
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ANDROID_JNILIBS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_BINARY_DIR}/bin/modules" "${ANDROID_JNILIBS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:adam-gui>" "${ANDROID_JNILIBS_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:adam-core>" "${ANDROID_JNILIBS_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" "${ANDROID_JNILIBS_DIR}/"
        DEPENDS adam-gui adam-core adam-module-asterix adam-module-can adam-module-network adam-module-recrep adam-module-serial
        COMMENT "Staging all native libraries and modules to ${ANDROID_JNILIBS_DIR}"
    )
endif()
