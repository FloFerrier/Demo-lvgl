#include "ui_utils.h"
#include <stdlib.h>
#include <assert.h>

void create_box(lv_obj_t *parent, enum type_graphic_e flag_graphic) {
    assert(NULL != parent);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_opa(&style, LV_OPA_COVER);
    lv_style_set_bg_color(&style, lv_palette_lighten(LV_PALETTE_GREY, 1));

    lv_obj_t *obj = lv_btn_create(parent);
    lv_obj_add_event_cb(obj, box_btn_event, LV_EVENT_ALL, NULL);
    lv_obj_set_size(obj, lv_pct(44), lv_pct(44));
    lv_obj_add_style(obj, &style, 0);

    lv_obj_t *name = lv_label_create(obj);
    lv_obj_align(name, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t *value = lv_label_create(obj);
    lv_obj_align(value, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_font(value, &lv_font_montserrat_22, 0);
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

void line_chart(lv_obj_t *parent, MYSQL *con, enum type_graphic_e flag_graphic) {
    assert((NULL != parent) && (NULL != con));

    lv_obj_t *chart = lv_chart_create(parent);
    lv_obj_set_size(chart, 200, 150);
    lv_obj_center(chart);
    lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER);   /*Show lines and points too*/

    /*Add two data series*/
    lv_chart_series_t *serie = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

    int *data = malloc(sizeof(int)*DATA_NB_ELMT_MAX);
    int nb_elmt = 0;

    get_data_graphic(con, flag_graphic, data, &nb_elmt);
    lv_chart_set_point_count(chart, nb_elmt);

    /*Set the next points on 'serie'*/
    for(int idx=0; idx < nb_elmt; idx++) {
        lv_chart_set_next_value2(chart, serie, idx, data[idx]);
    }

    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 6, 5, true, 60);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, nb_elmt-1);

    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 10, 5, 6, 1, true, 30);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, min_value(data, nb_elmt), max_value(data, nb_elmt));

    lv_chart_refresh(chart);
    free(data);
}

void tab_view(lv_obj_t *parent, MYSQL *database) {
    assert(NULL != parent);

    lv_obj_t *tabview;
    tabview = lv_tabview_create(parent, LV_DIR_TOP, lv_pct(20));

    lv_obj_t *temperature = lv_tabview_add_tab(tabview, "Temperature");
    lv_obj_t *humidity = lv_tabview_add_tab(tabview, "Humidity");
    lv_obj_t *pressure = lv_tabview_add_tab(tabview, "Pressure");
    lv_obj_t *eco2 = lv_tabview_add_tab(tabview, "eCO2");

    line_chart(temperature, database,TYPE_GRAPHIC_TEMPERATURE);
    lv_obj_center(temperature);

    line_chart(humidity, database, TYPE_GRAPHIC_HUMIDITY);
    lv_obj_center(humidity);

    line_chart(pressure, database, TYPE_GRAPHIC_PRESSURE);
    lv_obj_center(pressure);

    lv_obj_t *label = lv_label_create(eco2);
    lv_label_set_text(label, "NO DATA");
    lv_obj_center(label);

    back_btn(parent);
}

void back_btn(lv_obj_t *parent) {
    assert(NULL != parent);

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