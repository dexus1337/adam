
# CMakeLists.txt for the gui application

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


# Grab all source files for the sdk and tests automatically
file(GLOB_RECURSE adam_gui_app_source_files         "source/*cpp")
file(GLOB_RECURSE adam_gui_app_test_source_files    "test/*cpp")

set(app_resources "../../resources/resources.rc")


add_executable(adam-gui 
    ${adam_gui_app_source_files}
    ${imgui_sources}
    ${app_resources}
)

target_include_directories(adam-gui PRIVATE
    "${imgui_dir}"
    "${imgui_dir}/backends"
    "../../modules/recrep/include"
)

target_link_libraries(adam-gui PRIVATE
    adam-sdk
    SDL3::SDL3
    OpenGL::GL
)

# Set the output directory for the executable
set_target_properties(adam-gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)