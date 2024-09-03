#
# LIB_PREFIX: LWBTN
#
# This file provides set of variables for end user
# and also generates one (or more) libraries, that can be added to the project using target_link_libraries(...)
#
# Before this file is included to the root CMakeLists file (using include() function), user can set some variables:
#
# LWBTN_OPTS_FILE: If defined, it is the path to the user options file. If not defined, one will be generated for you automatically
# LWBTN_COMPILE_OPTIONS: If defined, it provide compiler options for generated library.
# LWBTN_COMPILE_DEFINITIONS: If defined, it provides "-D" definitions to the library build
#

# Custom include directory
set(LWBTN_CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib_inc)

# Library core sources
set(lwbtn_core_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/lwbtn/lwbtn.c
)

# Setup include directories
set(lwbtn_include_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    ${LWBTN_CUSTOM_INC_DIR}
)

# Register library to the system
add_library(lwbtn)
target_sources(lwbtn PRIVATE ${lwbtn_core_SRCS})
target_include_directories(lwbtn PUBLIC ${lwbtn_include_DIRS})
target_compile_options(lwbtn PRIVATE ${LWBTN_COMPILE_OPTIONS})
target_compile_definitions(lwbtn PRIVATE ${LWBTN_COMPILE_DEFINITIONS})

# Create config file if user didn't provide one info himself
if(NOT LWBTN_OPTS_FILE)
    message(STATUS "Using default lwbtn_opts.h file")
    set(LWBTN_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/src/include/lwbtn/lwbtn_opts_template.h)
else()
    message(STATUS "Using custom lwbtn_opts.h file from ${LWBTN_OPTS_FILE}")
endif()

configure_file(${LWBTN_OPTS_FILE} ${LWBTN_CUSTOM_INC_DIR}/lwbtn_opts.h COPYONLY)