add_library(button
    button.c
)

target_include_directories(button PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)