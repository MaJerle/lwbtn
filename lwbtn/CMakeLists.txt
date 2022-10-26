cmake_minimum_required(VERSION 3.22)

# Register library to the system
add_library(lwbtn INTERFACE)
target_sources(lwbtn INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/lwbtn/lwbtn.c)
target_include_directories(lwbtn INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/include)
