add_executable(Src
    main.c
)


target_include_directories(Src PUBLIC ../Inc)

target_link_libraries(Src
    pico_stdlib
    hardware_i2c

    lib
)

pico_add_extra_outputs(Src)
