cmake_minimum_required(VERSION 3.14)
include(FetchContent)

project(MyProject LANGUAGES C)

# Build an executable called "MyFirmware"
add_executable(MyFirmware
    src/main.c
    src/img_icon_temperature.c
    src/img_icon_humidity.c
    src/img_icon_pressure.c
    src/img_icon_eco2.c)

# Specify path to own LVGL config header
set(LV_CONF_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lv_conf.h
    CACHE STRING "" FORCE)

set(LV_LVGL_H_INCLUDE_SIMPLE)

find_package(SDL2 REQUIRED SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

# Fetch LVGL from GitHub
FetchContent_Declare(lvgl GIT_REPOSITORY https://github.com/lvgl/lvgl.git GIT_TAG v8.2.0)
FetchContent_MakeAvailable(lvgl)

# Fetch LV_DRIVERS from GitHub
FetchContent_Declare(lv_drivers GIT_REPOSITORY https://github.com/lvgl/lv_drivers GIT_TAG v8.2.0)
FetchContent_MakeAvailable(lv_drivers)

# The target "MyFirmware" depends on LVGL and drivers
target_link_libraries(MyFirmware PRIVATE lvgl::lvgl lvgl::drivers ${SDL2_LIBRARIES})