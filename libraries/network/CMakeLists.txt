set(network_tools_dir ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE network_tools_source_files "source/*.cpp")

add_library(adam-network STATIC ${network_tools_source_files})

target_include_directories(adam-network PUBLIC
    "include"
)

target_link_libraries(adam-network PUBLIC
    adam-core
)

if(WIN32)
    target_link_libraries(adam-network PRIVATE wininet)
else()
    target_link_libraries(adam-network PRIVATE curl)
endif()
