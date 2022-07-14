#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <mysql.h>
#include <SDL2/SDL.h>

#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/

#include "lvgl.h"
#include "../lv_drivers-src/display/fbdev.h"
#include "../lv_drivers-src/sdl/sdl.h"

#define DISP_BUF_SIZE (SDL_HOR_RES * SDL_VER_RES)

#define BUFFER_CHARACTERS_MAX 255
#define DATA_NB_ELMT_MAX      255

#define MYSQL_USER_NAME      "openpill-dev"
#define MYSQL_USER_PASSWORD  "openpill"
#define MYSQL_DATABASE       "Openpill"
#define MYSQL_TABLE_SELECTED "13July2022"

enum type_graphic_e {
    TYPE_GRAPHIC_TEMPERATURE,
    TYPE_GRAPHIC_HUMIDITY,
    TYPE_GRAPHIC_PRESSURE,
    TYPE_GRAPHIC_ECO2,
};

static lv_obj_t *timestamp;
static MYSQL *con;

static int g_id;
static int g_timestamp;
static int g_temperature;
static int g_humidity;
static int g_pressure;
static int g_eco2;

static void box_btn_event(lv_event_t *event);
static void back_btn_event(lv_event_t *event);

static void page_home_open(void);
static void page_graphics_open(enum type_graphic_e flag_graphic);

static void create_box(lv_obj_t *parent, enum type_graphic_e flag_graphic);
static void line_chart(lv_obj_t *parent, enum type_graphic_e flag_graphic);
static void tab_view(lv_obj_t *parent);
static void back_btn(lv_obj_t *parent);

static bool mysql_command(MYSQL *con, const char *cmd, char *res, size_t *size_res);

static void get_data_graphic(MYSQL *con, enum type_graphic_e type, int *data, int *nb_elmt);
static void get_data_dashboard(MYSQL *con, int *data, int *nb_elmt);

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
        fprintf(stderr, "%s\n", mysql_error(con));
    }

    if(NULL == mysql_real_connect(con, "localhost", MYSQL_USER_NAME, MYSQL_USER_PASSWORD,
        NULL, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
    }

    /* Select correct database */
    if(0 != mysql_select_db(con, MYSQL_DATABASE)) {
        fprintf(stderr, "Error to select %s\n", MYSQL_DATABASE);
        mysql_close(con);
    }
    LV_LOG_USER("%s is selected with success", MYSQL_DATABASE);

    /* Page at startup */
    page_home_open();

    struct tm *current_time;
    time_t epoch;

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_tick_inc(600);
        lv_task_handler();
        epoch = time(NULL);
        current_time = gmtime(&epoch);
        lv_label_set_text_fmt(timestamp, "TIMESTAMP %02d/%02d/%02d %d:%02d:%02d",
            current_time->tm_mday, current_time->tm_mon + 1, current_time->tm_year + 1900,\
            current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
        //usleep(5000);
    }

    mysql_close(con);
    return 0;
}

void page_home_open(void) {
    int *data = malloc(sizeof(int)*DATA_NB_ELMT_MAX);
    int nb_elmt = 0;
    get_data_dashboard(con, data, &nb_elmt);
    if(6 != nb_elmt) {
        LV_LOG_USER("data from databse may to be corrupted ...");
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
}

void create_box(lv_obj_t *parent, enum type_graphic_e flag_graphic) {
    if(NULL == parent) {
        LV_LOG_USER("parent pointer is NULL");
        return;
    }
    static lv_style_t style_box;
    lv_style_init(&style_box);
    lv_style_set_bg_opa(&style_box, LV_OPA_COVER);
    lv_style_set_bg_color(&style_box, lv_palette_lighten(LV_PALETTE_GREY, 1));

    lv_obj_t *obj = lv_btn_create(parent);
    lv_obj_add_event_cb(obj, box_btn_event, LV_EVENT_ALL, NULL);
    lv_obj_set_size(obj, lv_pct(44), lv_pct(44));
    lv_obj_add_style(obj, &style_box, 0);

    lv_obj_t *name = lv_label_create(obj);
    lv_obj_align(name, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t *value = lv_label_create(obj);
    lv_obj_align(value, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_t *icon = lv_img_create(obj);
    lv_obj_align(icon, LV_ALIGN_TOP_RIGHT, 0, 0);

    switch(flag_graphic) {
        case TYPE_GRAPHIC_TEMPERATURE :
            lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_1);
            LV_IMG_DECLARE(img_icon_temperature);
            lv_img_set_src(icon, &img_icon_temperature);
            lv_label_set_text_fmt(value, "%d °C", g_temperature);
            lv_label_set_text(name, "TEMPERATURE");
            break;
        case TYPE_GRAPHIC_HUMIDITY :
            lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_2);
            LV_IMG_DECLARE(img_icon_humidity);
            lv_img_set_src(icon, &img_icon_humidity);
            lv_label_set_text_fmt(value, "%d rH", g_humidity);
            lv_label_set_text(name, "HUMIDITY");
            break;
        case TYPE_GRAPHIC_PRESSURE :
            lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_3);
            LV_IMG_DECLARE(img_icon_pressure);
            lv_img_set_src(icon, &img_icon_pressure);
            lv_label_set_text_fmt(value, "%d hPa", g_pressure);
            lv_label_set_text(name, "PRESSURE");
            break;
        case TYPE_GRAPHIC_ECO2 :
            lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_4);
            LV_IMG_DECLARE(img_icon_eco2);
            lv_img_set_src(icon, &img_icon_eco2);
            lv_label_set_text_fmt(value, "%d ppm", g_eco2);
            lv_label_set_text(name, "eCO2");
            break;
        default:
            break;
    }
}

void box_btn_event(lv_event_t *event) {
    if(NULL == event) {
        LV_LOG_USER("event pointer is NULL");
        return;
    }
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

void page_graphics_open(enum type_graphic_e flag_graphic) {
    lv_obj_t *page = lv_obj_create(lv_scr_act());
    static lv_style_t style;
    lv_style_init(&style);
    lv_obj_set_size(page, lv_pct(100), lv_pct(100));
    lv_obj_add_style(page, &style, 0);
    tab_view(page);
}

void back_btn_event(lv_event_t *event) {
    if(NULL == event) {
        LV_LOG_USER("event pointer is NULL");
        return;
    }
    lv_event_code_t code = lv_event_get_code(event);
    if (LV_EVENT_CLICKED == code) {
        LV_LOG_USER("Click on back button => Open Home Page");
        page_home_open();
    }
}

void line_chart(lv_obj_t *parent, enum type_graphic_e flag_graphic) {
    if(NULL == parent) {
        LV_LOG_USER("parent pointer is NULL");
        return;
    }
    lv_obj_t *chart = lv_chart_create(parent);
    lv_obj_set_size(chart, 200, 150);
    lv_obj_center(chart);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   /*Show lines and points too*/

    /*Add two data series*/
    lv_chart_series_t *serie1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 6, 5, true, 40);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 10, 5, 10, 1, true, 30);

    int *data = malloc(sizeof(int)*DATA_NB_ELMT_MAX);
    int nb_elmt = 0;
    get_data_graphic(con, flag_graphic, data, &nb_elmt);

    /*Set the next points on 'serie1'*/
    for(int idx=0; idx < nb_elmt; idx++) {
        lv_chart_set_next_value(chart, serie1, data[idx]);
    }

    lv_chart_refresh(chart);
    free(data);
}

void tab_view(lv_obj_t *parent) {
    if(NULL == parent) {
        LV_LOG_USER("parent pointer is NULL");
        return;
    }
    lv_obj_t *tabview;
    tabview = lv_tabview_create(parent, LV_DIR_TOP, lv_pct(20));
    //lv_menu_set_mode_root_back_btn(parent, LV_MENU_ROOT_BACK_BTN_ENABLED);

    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Temperature");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Humidity");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Pressure");
    lv_obj_t *tab4 = lv_tabview_add_tab(tabview, "eCO2");

    line_chart(tab1, TYPE_GRAPHIC_TEMPERATURE);
    lv_obj_center(tab1);

    line_chart(tab2, TYPE_GRAPHIC_HUMIDITY);
    lv_obj_center(tab2);

    line_chart(tab3, TYPE_GRAPHIC_PRESSURE);
    lv_obj_center(tab3);

    lv_obj_t *label = lv_label_create(tab4);
    lv_label_set_text(label, "NO DATA");
    lv_obj_center(label);

    back_btn(parent);
}

void back_btn(lv_obj_t *parent) {
    if(NULL == parent) {
        LV_LOG_USER("parent pointer is NULL");
        return;
    }
    static lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn, 50);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0xFFFFFFFF));
    lv_obj_t *back_btn;
    back_btn = lv_btn_create(parent);
    lv_obj_add_style(back_btn, &style_btn, 0);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 3);
    lv_style_set_line_rounded(&style_line, true);
    lv_obj_t *obj_line = lv_line_create(back_btn);
    lv_obj_add_style(obj_line, &style_line, 0);
    static lv_point_t points[] = {{10, 0}, {0, 5}, {10, 10}};
    lv_line_set_points(obj_line, points, 3);
    lv_obj_center(obj_line);
    lv_obj_add_event_cb(back_btn, back_btn_event, LV_EVENT_ALL, NULL);
}

bool mysql_command(MYSQL *con, const char *cmd, char *result, size_t *size_result) {
    if((NULL == con) || (NULL == cmd) || (NULL == result)) {
        LV_LOG_USER("con or cmd or res is NULL");
        return false;
    }
    if(mysql_query(con, cmd)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        return false;
    }
    MYSQL_RES *res = mysql_store_result(con);
    if(NULL == res) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        return false;
    }
    MYSQL_ROW row;
    uint num_fields = mysql_num_fields(res);
    //LV_LOG_USER("num_fields: %d", num_fields);
    while((row = mysql_fetch_row(res))) {
        for(int i = 0; i < num_fields; i++) {
            strncat(result, row[i], BUFFER_CHARACTERS_MAX);
            strcat(result, " ");
            //LV_LOG_USER("%s ", row[i] ? row[i] : "NULL");
        }
        //LV_LOG_USER("\n");
    }
    *size_result = strlen(result);
    mysql_free_result(res);

    if(0 != *size_result)
        return false;
    else
        return true;
}

/* Get all data for a specific field */
void get_data_graphic(MYSQL *con, enum type_graphic_e type, int *data, int *nb_elmt) {
    if((NULL == con) || (NULL == data) || (NULL == nb_elmt)) {
        LV_LOG_USER("con or data or nb_elmt is NULL");
        return;
    }
    char pType[15] = "";
    switch (type) {
        case TYPE_GRAPHIC_TEMPERATURE :
            strcpy(pType, "temperature");
            break;
        case TYPE_GRAPHIC_HUMIDITY :
            strcpy(pType, "humidity");
            break;
        case TYPE_GRAPHIC_PRESSURE :
            strcpy(pType, "pressure");
            break;
        case TYPE_GRAPHIC_ECO2 :
            strcpy(pType, "eco2");
            break;
        default:
            break;
    }
    char *cmd = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    snprintf(cmd, BUFFER_CHARACTERS_MAX, "SELECT %s FROM %s", pType, MYSQL_TABLE_SELECTED);
    char *res = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    memset(res, '\0', BUFFER_CHARACTERS_MAX+1);
    size_t size_res = 0;
    mysql_command(con, cmd, res, &size_res);
    LV_LOG_USER("Raw [%d] %s", size_res, res);

    // Returns first token
    char *save_ptr;
    char *token = strtok_r(res, " ", &save_ptr);

    *nb_elmt = 0;
    while(NULL != token) {
        *nb_elmt += 1;
        //LV_LOG_USER("Token [%d] %s", *nb_elmt, token);
        data[*nb_elmt-1] = atoi(token);
        token = strtok_r(NULL, " ", &save_ptr);
    }
    free(cmd);
    free(res);
}

/* Get the lastest data */
void get_data_dashboard(MYSQL *con, int *data, int *nb_elmt) {
    if((NULL == con) || (NULL == data) || (NULL == nb_elmt) ) {
        LV_LOG_USER("con or data or nb_elmt is NULL");
        return;
    }
    char *cmd = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    snprintf(cmd, BUFFER_CHARACTERS_MAX, "SELECT * FROM %s ORDER BY id DESC LIMIT 1", MYSQL_TABLE_SELECTED);
    char *res = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    memset(res, '\0', BUFFER_CHARACTERS_MAX+1);
    size_t size_res = 0;
    mysql_command(con, cmd, res, &size_res);
    LV_LOG_USER("Raw [%d] %s", size_res, res);

    // Returns first token
    char *save_ptr;
    char *token = strtok_r(res, " ", &save_ptr);

    *nb_elmt = 0;
    while(NULL != token) {
        *nb_elmt += 1;
        //LV_LOG_USER("Token [%d] %s", *nb_elmt, token);
        data[*nb_elmt-1] = atoi(token);
        token = strtok_r(NULL, " ", &save_ptr);
    }

    free(cmd);
    free(res);
}