#ifndef UI_UTILS_H
#define UI_UTILS_H

#include "lvgl.h"
#include "db_utils.h"
#include <mysql.h>

#define DATA_NB_ELMT_MAX      20

extern int g_id;
extern int g_timestamp;
extern int g_temperature;
extern int g_humidity;
extern int g_pressure;
extern int g_eco2;

extern void box_btn_event(lv_event_t *event);
extern void back_btn_event(lv_event_t *event);

void create_box(lv_obj_t *parent, enum type_graphic_e flag_graphic);
void line_chart(lv_obj_t *parent, MYSQL *con, enum type_graphic_e flag_graphic);
void tab_view(lv_obj_t *parent, MYSQL *database);
void back_btn(lv_obj_t *parent);

#endif /* UI_UTILS_H */