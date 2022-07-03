#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <SDL2/SDL.h>

#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/

#include "lvgl.h"
#include "../lv_drivers-src/display/fbdev.h"
#include "../lv_drivers-src/sdl/sdl.h"

#define DISP_BUF_SIZE (SDL_HOR_RES * SDL_VER_RES)

int main(void) {

    printf("DEMO-LVGL\r\n");

    /*LittlevGL init*/
    lv_init();
 
    /*Linux frame buffer device init*/
    //fbdev_init();
    /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    sdl_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);
 
    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = sdl_display_flush;
    disp_drv.hor_res = SDL_HOR_RES;
    disp_drv.ver_res = SDL_VER_RES;
    lv_disp_drv_register(&disp_drv);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_flex_flow(&style, LV_FLEX_FLOW_ROW_WRAP);
    lv_style_set_flex_main_place(&style, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_style_set_layout(&style, LV_LAYOUT_FLEX);

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_center(cont);
    lv_obj_add_style(cont, &style, 0);

    for(uint32_t index = 0; index < 4; index++) {
        lv_obj_t *obj = lv_obj_create(cont);
        lv_obj_set_size(obj, lv_pct(40), lv_pct(40));
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_t *name = lv_label_create(obj);
        lv_obj_t *value = lv_label_create(obj);
        switch(index) {
            case 0:
                lv_label_set_text(name, "TEMPERATURE");
                lv_label_set_text_fmt(value, "%d°C", 25);
                break;
            case 1:
                lv_label_set_text(name, "HUMIDITY");
                lv_label_set_text_fmt(value, "%d\%rH", 50);
                break;
            case 2:
                lv_label_set_text(name, "PRESSURE");
                lv_label_set_text(value, "990hPa");
                lv_label_set_text_fmt(value, "%dhPa", 990);
                break;
            case 3:
                lv_label_set_text(name, "eCO2");
                lv_label_set_text_fmt(value, "%dppm", 800);
                break;
            default:
                break;
        }
        lv_obj_align(name, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        lv_obj_center(value);
    }

    lv_obj_t *timestamp = lv_label_create(cont);
    lv_obj_align(timestamp, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_tick_inc(5);
        lv_task_handler();
        time_t epoch = time(NULL);
        struct tm *current_time;
        current_time = gmtime(&epoch);
        lv_label_set_text_fmt(timestamp, "TIMESTAMP %02d/%02d/%02d %d:%02d:%02d",
            current_time->tm_mday, current_time->tm_mon + 1, current_time->tm_year + 1900,\
            current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
        sleep(1);
    }

    return 0;
}