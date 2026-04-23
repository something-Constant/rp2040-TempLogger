add_library(fatfs
    ./diskio.c
    ./ff.c
    ./ffsystem.c
    ./ffunicode.c
)

target_include_directories(fatfs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fatfs
    physical-api
)
