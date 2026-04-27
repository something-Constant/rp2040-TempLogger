add_library(BCD_Conversion
    ./BCD_Conversion.c
)

target_include_directories(BCD_Conversion PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)