cmake_minimum_required(VERSION 3.22)

# Register library to the system
add_library(lwbtn INTERFACE)
target_sources(lwbtn INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/lwbtn/lwbtn.c)
target_include_directories(lwbtn INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/include)

# Create config file
if(DEFINED LWBTN_OPTS_DIR AND NOT EXISTS ${LWBTN_OPTS_DIR}/lwbtn_opts.h)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/src/include/lwbtn/lwbtn_opts_template.h ${LWBTN_OPTS_DIR}/lwbtn_opts.h COPYONLY)
endif()
