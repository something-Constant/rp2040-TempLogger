# include(../pico_sdk_import.cmake)

# add_executable(core
# Src/main.c
# )

# include_directories(core
# Inc
# )
# target_include_directories(core PUBLIC Inc)

# target_link_libraries(core
# pico_stdlib
# hardware_i2c
# lib
# )

# pico_add_extra_outputs(core)
# include_directories(main
#     Inc
# )
add_subdirectory(Src)
add_subdirectory(Inc)

