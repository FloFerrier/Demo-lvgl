#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <mysql.h>
#include <SDL2/SDL.h>
#include <assert.h>

#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/

#include "lvgl.h"
#include "../lv_drivers-src/display/fbdev.h"
#include "../lv_drivers-src/sdl/sdl.h"

#include "ui_utils.h"
#include "db_utils.h"
#include "gen_header.h"

#define DISP_BUF_SIZE (SDL_HOR_RES * SDL_VER_RES)

static lv_obj_t *timestamp;

MYSQL *con;

int g_id;
int g_timestamp;
int g_temperature;
int g_humidity;
int g_pressure;
int g_eco2;

char g_tablename[10] = "";

void box_btn_event(lv_event_t *event);
void back_btn_event(lv_event_t *event);

static bool page_home_open(void);
static void page_error_database(enum error_database_e error);
static void page_graphics_open(enum type_graphic_e flag_graphic);

static void infinite_loop(void);

int main(void) {

    LV_LOG_USER("DEMO-LVGL");

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

    /*Initialize and register an input driver*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = sdl_mouse_read;
    /*Register the driver in LVGL and save the created input device object*/
    lv_indev_t *my_indev = lv_indev_drv_register(&indev_drv);

    LV_LOG_USER("MySQL client version: %s", mysql_get_client_info());
    con = mysql_init(NULL);
    if(NULL == con) {
        LV_LOG_ERROR("%s\n", mysql_error(con));
        page_error_database(ERROR_DATABASE_INIT);
    }

    if(NULL == mysql_real_connect(con, "localhost", MYSQL_USER_NAME, MYSQL_USER_PASSWORD,
        NULL, 0, NULL, 0)) {
        LV_LOG_ERROR("%s\n", mysql_error(con));
        mysql_close(con);
        page_error_database(ERROR_DATABASE_CONNECT);
    }

    /* Select correct database */
    if(0 != mysql_select_db(con, MYSQL_DATABASE)) {
        LV_LOG_ERROR("Error to select %s\n", MYSQL_DATABASE);
        mysql_close(con);
        page_error_database(ERROR_DATABASE_SELECT);
    }
    LV_LOG_USER("%s is selected with success", MYSQL_DATABASE);

    struct tm *current_time;
    time_t epoch;
    epoch = time(NULL);
    current_time = gmtime(&epoch);

    char *encoded_time = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    strftime(g_tablename, BUFFER_CHARACTERS_MAX, "%d%B%Y", current_time);

    /* Page at startup */
    if(!page_home_open()) {
        LV_LOG_ERROR("data from database may to be corrupted ...");
        page_error_database(ERROR_DATABASE_DATA);
    }

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_tick_inc(600);
        lv_task_handler();
        epoch = time(NULL);
        current_time = gmtime(&epoch);

        strftime(encoded_time, BUFFER_CHARACTERS_MAX, "%d/%m/%Y %H:%M:%S", current_time);
        lv_label_set_text_fmt(timestamp, "TIMESTAMP %s", encoded_time);
        //usleep(5000);
    }

    free(encoded_time);
    mysql_close(con);
    return 0;
}

bool page_home_open(void) {
    int *data = malloc(sizeof(int)*DATA_NB_ELMT_MAX);
    int nb_elmt = 0;
    get_data_dashboard(con, data, &nb_elmt);
    if(6 != nb_elmt) {
        return false;
    }
    else {
        g_id          = data[0];
        g_timestamp   = data[1];
        g_temperature = data[2];
        g_humidity    = data[3];
        g_pressure    = data[4];
        g_eco2        = data[5];
        LV_LOG_USER("ID= %d", g_id);
        LV_LOG_USER("Timestamp= %d", g_timestamp);
        LV_LOG_USER("Temperature= %d", g_temperature);
        LV_LOG_USER("Humidity= %d", g_humidity);
        LV_LOG_USER("Pressure= %d", g_pressure);
        LV_LOG_USER("eCO2= %d", g_eco2);
    }
    free(data);

    static lv_style_t style_page;
    lv_style_init(&style_page);
    lv_style_set_flex_flow(&style_page, LV_FLEX_FLOW_ROW_WRAP);
    lv_style_set_flex_main_place(&style_page, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_style_set_layout(&style_page, LV_LAYOUT_FLEX);

    lv_obj_t *obj_page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj_page, lv_pct(100), lv_pct(100));
    lv_obj_center(obj_page);
    lv_obj_add_style(obj_page, &style_page, 0);

    /* Define specific box */
    create_box(obj_page, TYPE_GRAPHIC_TEMPERATURE);
    create_box(obj_page, TYPE_GRAPHIC_HUMIDITY);
    create_box(obj_page, TYPE_GRAPHIC_PRESSURE);
    create_box(obj_page, TYPE_GRAPHIC_ECO2);

    timestamp = lv_label_create(obj_page);
    lv_obj_align(timestamp, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    return true;
}

void box_btn_event(lv_event_t *event) {
    assert(NULL != event);

    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *btn = lv_event_get_target(event);

    if(LV_EVENT_PRESSED == code) {
        /* Open Graphics Page */
        LV_LOG_USER("Click on box => Open Graphics Page");

        if(LV_OBJ_FLAG_USER_1 & btn->flags) {
            LV_LOG_USER("Click on Box1 - Top Left");
            page_graphics_open(TYPE_GRAPHIC_TEMPERATURE);
        }
        else if(LV_OBJ_FLAG_USER_2 & btn->flags) {
            LV_LOG_USER("Click on Box2 - Top Right");
            page_graphics_open(TYPE_GRAPHIC_HUMIDITY);
        }
        else if(LV_OBJ_FLAG_USER_3 & btn->flags) {
            LV_LOG_USER("Click on Box3 - Bottom Left");
            page_graphics_open(TYPE_GRAPHIC_PRESSURE);
        }
        else if(LV_OBJ_FLAG_USER_4 & btn->flags) {
            LV_LOG_USER("Click on Box4 - Bottom Right");
            page_graphics_open(TYPE_GRAPHIC_ECO2);
        }
    }
}

void page_error_database(enum error_database_e error) {
    static lv_style_t style;
    lv_style_init(&style);

    lv_obj_t *obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, &style, 0);
    lv_obj_set_size(obj, lv_pct(100), lv_pct(100));

    lv_obj_t *text = lv_label_create(obj);
    lv_obj_center(text);

    char *text_error = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    switch(error) {
        case ERROR_DATABASE_INIT :
            strncpy(text_error, "ERROR_DATABASE_INIT", BUFFER_CHARACTERS_MAX);
            break;
        case ERROR_DATABASE_CONNECT :
            strncpy(text_error, "ERROR_DATABASE_CONNECT", BUFFER_CHARACTERS_MAX);
            break;
        case ERROR_DATABASE_SELECT :
            strncpy(text_error, "ERROR_DATABASE_SELECT", BUFFER_CHARACTERS_MAX);
            break;
        case ERROR_DATABASE_DATA:
            strncpy(text_error, "ERROR_DATABASE_DATA", BUFFER_CHARACTERS_MAX);
            break;
        default:
            strncpy(text_error, "", BUFFER_CHARACTERS_MAX);
            break;
    }
    lv_label_set_text(text, text_error);
    free(text_error);

    infinite_loop();
}

void infinite_loop(void) {
    while(1) {
        lv_tick_inc(600);
        lv_task_handler();
    }
}

void page_graphics_open(enum type_graphic_e flag_graphic) {
    lv_obj_t *page = lv_obj_create(lv_scr_act());
    static lv_style_t style;
    lv_style_init(&style);
    lv_obj_set_size(page, lv_pct(100), lv_pct(100));
    lv_obj_add_style(page, &style, 0);
    tab_view(page, con);
}

void back_btn_event(lv_event_t *event) {
    assert(NULL != event);

    lv_event_code_t code = lv_event_get_code(event);
    if (LV_EVENT_CLICKED == code) {
        LV_LOG_USER("Click on back button => Open Home Page");
        page_home_open();
    }
}