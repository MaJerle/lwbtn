# CMake include file

# Add more sources
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test_lwbtn_main.c
)

# Options file
set(LWDBTNOPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/lwdtc_opts.h)