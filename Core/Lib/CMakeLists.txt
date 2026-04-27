
add_subdirectory("ssd1306")
add_subdirectory("sd-card")
add_subdirectory("DS3231-driver(RTC)")
add_subdirectory("BCD_Conversion")

include_directories(${PROJECT_NAME}
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_SOURCE_DIR}/moving_average_filter
   ${CMAKE_SOURCE_DIR}/median_filter
   ${CMAKE_SOURCE_DIR}/font_5x7
)