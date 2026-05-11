
# CMakeLists.txt for the gui application

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(imgui_dir $ENV{IMGUI_ROOT})
if(NOT imgui_dir)
    message(FATAL_ERROR "IMGUI_ROOT environment variable is not set. Please set it to the path of Dear ImGui.")
endif()


find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)


# Grab the core ImGui files and the SDL2/OpenGL3 backends
file(GLOB imgui_sources
    "${imgui_dir}/*.cpp"
    "${imgui_dir}/backends/imgui_impl_sdl2.cpp"
    "${imgui_dir}/backends/imgui_impl_opengl3.cpp"
)


# Grab all source files for the sdk and tests automatically
file( GLOB_RECURSE  adam_gui_app_source_files       "source/*cpp"   )
file( GLOB_RECURSE  adam_gui_app_test_source_files   "test/*cpp"    ) 


add_executable(adam-gui 
    ${adam_gui_app_source_files}
    ${imgui_sources})

target_include_directories(adam-gui PRIVATE
    "${imgui_dir}"
    "${imgui_dir}/backends"
)

target_link_libraries(adam-gui PRIVATE
    adam-sdk
    ${SDL2_LIBRARIES}
    OpenGL::GL
)

# Set the output directory for the executable
set_target_properties(adam-gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)