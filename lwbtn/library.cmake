# Library core sources
set(lwbtn_core_SRCS 
    ${CMAKE_CURRENT_LIST_DIR}/src/lwbtn/lwbtn.c
)

# Setup include directories
set(lwbtn_include_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/src/include
)

# Register library to the system
add_library(lwbtn INTERFACE)
target_sources(lwbtn INTERFACE ${lwbtn_core_SRCS})
target_include_directories(lwbtn INTERFACE ${lwbtn_include_DIRS})
target_compile_options(lwbtn PRIVATE ${LWBTN_COMPILE_OPTIONS})
target_compile_definitions(lwbtn PRIVATE ${LWBTN_COMPILE_DEFINITIONS})

# Create config file
if(DEFINED LWBTN_OPTS_DIR AND NOT EXISTS ${LWBTN_OPTS_DIR}/lwbtn_opts.h)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/src/include/lwbtn/lwbtn_opts_template.h ${LWBTN_OPTS_DIR}/lwbtn_opts.h COPYONLY)
endif()