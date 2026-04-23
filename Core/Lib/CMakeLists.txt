
add_subdirectory(ssd1306)
# add_subdirectory(sd-card/fatfs)
# add_subdirectory(sd-card/physical-api)
add_subdirectory(sd-card)



include_directories(${PROJECT_NAME}
    ./
)