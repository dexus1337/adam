
# CMakeLists.txt for the gui application

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(imgui_dir $ENV{IMGUI_ROOT})
if(NOT imgui_dir)
    message(FATAL_ERROR "IMGUI_ROOT environment variable is not set. Please set it to the path of Dear ImGui.")
endif()


if(NOT ANDROID)
    find_package(OpenGL REQUIRED)
endif()

# Grab the core ImGui files and the SDL3/OpenGL3 backends
file(GLOB imgui_sources
    "${imgui_dir}/*.cpp"
    "${imgui_dir}/backends/imgui_impl_sdl3.cpp"
    "${imgui_dir}/backends/imgui_impl_opengl3.cpp"
)

if(WIN32)
    list(APPEND imgui_sources "${imgui_dir}/backends/imgui_impl_dx11.cpp")
endif()

# Grab all source files for the target and tests automatically
file(GLOB_RECURSE adam_gui_app_source_files         "source/*cpp")
file(GLOB_RECURSE adam_gui_app_test_source_files    "test/*cpp")

set(app_resources "../../resources/resources.rc")

if(ANDROID)
    add_library(adam-gui SHARED
        ${adam_gui_app_source_files}
        ${imgui_sources}
    )
else()
    add_executable(adam-gui 
        ${adam_gui_app_source_files}
        ${imgui_sources}
        ${app_resources}
    )
endif()

target_include_directories(adam-gui PRIVATE
    "${imgui_dir}"
    "${imgui_dir}/backends"
    "../../libraries/image/include"
    "../../modules/recrep/include"
)

target_link_libraries(adam-gui PRIVATE
    adam-core
    adam-imgui
    SDL3::SDL3
)

if(ANDROID)
    target_link_libraries(adam-gui PRIVATE
        GLESv3
        EGL
        log
        android
    )
else()
    target_link_libraries(adam-gui PRIVATE
        OpenGL::GL
    )
endif()

if(WIN32)
    target_link_libraries(adam-gui PRIVATE d3d11 dxgi d3dcompiler)
endif()

# Set the output directory for the target
set_target_properties(adam-gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(ANDROID)
    set(ANDROID_JNILIBS_DIR "${CMAKE_SOURCE_DIR}/android/app/src/main/jniLibs/${ANDROID_ABI}")
    add_custom_command(TARGET adam-gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ANDROID_JNILIBS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:adam-gui>"
            "${ANDROID_JNILIBS_DIR}/$<TARGET_FILE_NAME:adam-gui>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:adam-core>"
            "${ANDROID_JNILIBS_DIR}/$<TARGET_FILE_NAME:adam-core>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3>"
            "${ANDROID_JNILIBS_DIR}/$<TARGET_FILE_NAME:SDL3::SDL3>"
        COMMENT "Auto-staging native libraries to ${ANDROID_JNILIBS_DIR}"
    )
endif()