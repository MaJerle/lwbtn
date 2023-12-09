# 
# This file provides set of variables for end user
# and also generates one (or more) libraries, that can be added to the project using target_link_libraries(...)
#
# Before this file is included to the root CMakeLists file (using include() function), user can set some variables:
#
# LWBTN_OPTS_DIR: If defined, it should set the folder path where options file shall be generated.
# LWBTN_COMPILE_OPTIONS: If defined, it provide compiler options for generated library.
# LWBTN_COMPILE_DEFINITIONS: If defined, it provides "-D" definitions to the library build
#

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