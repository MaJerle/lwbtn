cmake_minimum_required(VERSION 3.22)

# Setup project
project(LwLibPROJECT)

if(NOT PROJECT_IS_TOP_LEVEL)
    add_subdirectory(lwbtn)
else()
    add_executable(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/dev/main.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/example_win32.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/test.c
    )
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/dev
        ${CMAKE_CURRENT_LIST_DIR}/examples
    )

    # Add subdir with lwbtn and link to project
    set(LWBTN_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/dev/lwbtn_opts.h)
    add_subdirectory(lwbtn)
    target_link_libraries(${PROJECT_NAME} PUBLIC lwbtn)

    # Add compile options to the library, which will propagate options to executable through public link
    target_compile_definitions(lwbtn PUBLIC WIN32 _DEBUG CONSOLE LWBTN_DEV)
    target_compile_options(lwbtn PRIVATE -Wall -Wextra -Wpedantic)
endif()
