cmake_minimum_required(VERSION 3.14)

project(ui-lvgl)

set(SOURCE_FILES
    main.c
    ui_utils.c
    db_utils.c
    img_icon_eco2.c
    img_icon_humidity.c
    img_icon_pressure.c
    img_icon_temperature.c)

# Specify path to own LVGL config header
set(LV_CONF_PATH
    ${INCLUDE_DIR}/lv_conf.h
    CACHE STRING "" FORCE)

set(LV_LVGL_H_INCLUDE_SIMPLE)

find_package(SDL2 REQUIRED SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

# Specify path to get MariaDB package
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_MODULES_DIR}")
find_package(MariaDBClient REQUIRED)

# Fetch LVGL from GitHub
FetchContent_Declare(lvgl GIT_REPOSITORY https://github.com/lvgl/lvgl.git GIT_TAG v8.2.0)
FetchContent_MakeAvailable(lvgl)

# Fetch LV_DRIVERS from GitHub
FetchContent_Declare(lv_drivers GIT_REPOSITORY https://github.com/lvgl/lv_drivers GIT_TAG v8.2.0)
FetchContent_MakeAvailable(lv_drivers)

add_executable(ui-lvgl ${SOURCE_FILES})

target_link_libraries(ui-lvgl PRIVATE MariaDBClient::MariaDBClient lvgl::lvgl lvgl::drivers ${SDL2_LIBRARIES})

set_target_properties(ui-lvgl PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BINARY_DIR}")