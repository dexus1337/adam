set(radar_tools_dir ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE radar_tools_source_files "source/*.cpp")

add_library(adam-radar STATIC ${radar_tools_source_files})

target_include_directories(adam-radar PUBLIC
    "include"
)

target_link_libraries(adam-radar PUBLIC
    adam-core
)