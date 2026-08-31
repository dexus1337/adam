# cmake/adam-sdl3.cmake
# Resolve SDL3 either through find_package (desktop prebuilts) or local source / FetchContent (Android / fallback)

if(TARGET SDL3::SDL3)
    return()
endif()

find_package(SDL3 QUIET)

if(NOT TARGET SDL3::SDL3)
    set(sdl3_source_dir $ENV{SDL3_SRC_DIR})
    if(NOT sdl3_source_dir)
        set(sdl3_source_dir "C:/Users/PC/Documents/git/others/sdl3-src")
    endif()

    if(EXISTS "${sdl3_source_dir}/CMakeLists.txt")
        message(STATUS "Using local SDL3 source: ${sdl3_source_dir}")
        add_subdirectory("${sdl3_source_dir}" "${CMAKE_BINARY_DIR}/_deps/sdl3-build" EXCLUDE_FROM_ALL)
    else()
        include(FetchContent)
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.2.8
        )
        FetchContent_MakeAvailable(SDL3)
    endif()
endif()

