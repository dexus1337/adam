set(image_tools_dir ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE image_tools_source_files "source/*.cpp")

add_library(adam-image-tools STATIC ${image_tools_source_files})

target_include_directories(adam-image-tools PUBLIC
    "include"
)

target_link_libraries(adam-image-tools PUBLIC
    adam-sdk
)
