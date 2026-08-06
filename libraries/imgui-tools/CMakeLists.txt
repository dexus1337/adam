# CMakeLists.txt for imgui-tools static library

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(imgui_dir $ENV{IMGUI_ROOT})
if(NOT imgui_dir)
    message(FATAL_ERROR "IMGUI_ROOT environment variable is not set. Please set it to the path of Dear ImGui.")
endif()

# We only need the headers to compile the tools, but we'll include SDL3 and OpenGL3 for consistency if needed
find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE imgui_tools_source_files "source/*.cpp")

add_library(adam-imgui-tools STATIC ${imgui_tools_source_files})

target_include_directories(adam-imgui-tools PUBLIC
    "include"
    "${imgui_dir}"
    "${imgui_dir}/backends"
)

target_link_libraries(adam-imgui-tools PUBLIC
    adam-sdk
    SDL3::SDL3
    OpenGL::GL
)

if(WIN32)
    target_link_libraries(adam-imgui-tools PRIVATE d3d11 dxgi d3dcompiler)
endif()
