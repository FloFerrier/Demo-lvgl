#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <mysql.h>
#include <stdbool.h>
#include "ui_utils.h"

#define BUFFER_CHARACTERS_MAX 255

enum type_graphic_e {
    TYPE_GRAPHIC_TEMPERATURE,
    TYPE_GRAPHIC_HUMIDITY,
    TYPE_GRAPHIC_PRESSURE,
    TYPE_GRAPHIC_ECO2,
};

enum error_database_e {
    ERROR_DATABASE_INIT,
    ERROR_DATABASE_CONNECT,
    ERROR_DATABASE_SELECT,
    ERROR_DATABASE_DATA,
};

extern char g_tablename[10];

int min_value(int *data, int nb_elmt);
int max_value(int *data, int nb_elmt);

bool mysql_command(MYSQL *con, const char *cmd, char *res, size_t *size_res);

void get_data_graphic(MYSQL *con, enum type_graphic_e type, int *data, int *nb_elmt);
void get_data_dashboard(MYSQL *con, int *data, int *nb_elmt);

#endif /* DB_UTILS_H */