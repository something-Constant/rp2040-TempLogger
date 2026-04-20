# add_executable(core
# Src/main.c
# )

include_directories(Src
    ./
)

# target_include_directories(core PUBLIC Inc)

# target_link_libraries(core
# pico_stdlib
# hardware_i2c
# # pico_binary_info

# lib
# )

# pico_add_extra_outputs(core)
add_subdirectory(ssd1306)
