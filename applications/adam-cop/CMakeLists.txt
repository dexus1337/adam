# CMakeLists.txt for the adam-cop (Common Operational Picture) application

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(imgui_dir $ENV{IMGUI_ROOT})
if(NOT imgui_dir)
    message(FATAL_ERROR "IMGUI_ROOT environment variable is not set. Please set it to the path of Dear ImGui.")
endif()

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

# Grab the core ImGui files and the SDL3/OpenGL3 backends
file(GLOB imgui_sources
    "${imgui_dir}/*.cpp"
    "${imgui_dir}/backends/imgui_impl_sdl3.cpp"
    "${imgui_dir}/backends/imgui_impl_opengl3.cpp"
)

if(WIN32)
    list(APPEND imgui_sources "${imgui_dir}/backends/imgui_impl_dx11.cpp")
endif()

# Grab all source files for the application
file(GLOB_RECURSE adam_cop_app_source_files "source/*.cpp")

set(app_resources "../../resources/resources.rc")

add_executable(adam-cop 
    ${adam_cop_app_source_files}
    ${imgui_sources}
    ${app_resources}
)

target_include_directories(adam-cop PRIVATE
    "${imgui_dir}"
    "${imgui_dir}/backends"
    "../../modules/recrep/include"
)

target_link_libraries(adam-cop PRIVATE
    adam-sdk
    adam-imgui-tools
    SDL3::SDL3
    OpenGL::GL
)

if(WIN32)
    target_link_libraries(adam-cop PRIVATE d3d11 dxgi d3dcompiler wininet windowscodecs)
endif()

# Set the output directory for the executable
set_target_properties(adam-cop PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
