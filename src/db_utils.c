#include "db_utils.h"
#include "ui_utils.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int min_value(int *data, int nb_elmt) {
    assert(NULL!=data);

    int tmp = data[0];
    for(int i=0; i<nb_elmt; i++) {
        if(data[i] < tmp) {
            tmp = data[i];
        }
    }
    return tmp;
}

int max_value(int *data, int nb_elmt) {
    assert(NULL!=data);

    int tmp = data[0];
    for(int i=0; i<nb_elmt; i++) {
        if(data[i] > tmp) {
            tmp = data[i];
        }
    }
    return tmp;
}

bool mysql_command(MYSQL *con, const char *cmd, char *result, size_t *size_result) {
    assert((NULL != con) && (NULL != cmd) && (NULL != result));

    LV_LOG_USER("[CMD] %s", cmd);
    if(mysql_query(con, cmd)) {
        LV_LOG_ERROR("%s\n", mysql_error(con));
        mysql_close(con);
        return false;
    }
    MYSQL_RES *res = mysql_store_result(con);
    if(NULL == res) {
        LV_LOG_ERROR("%s\n", mysql_error(con));
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
        }
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
    assert((NULL != con) && (NULL != data) && (NULL != nb_elmt));

    char *name = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    switch (type) {
        case TYPE_GRAPHIC_TEMPERATURE :
            strncpy(name, "temperature", BUFFER_CHARACTERS_MAX);
            break;
        case TYPE_GRAPHIC_HUMIDITY :
            strncpy(name, "humidity", BUFFER_CHARACTERS_MAX);
            break;
        case TYPE_GRAPHIC_PRESSURE :
            strncpy(name, "pressure", BUFFER_CHARACTERS_MAX);
            break;
        case TYPE_GRAPHIC_ECO2 :
            strncpy(name, "eco2", BUFFER_CHARACTERS_MAX);
            break;
        default:
            strncpy(name, "", BUFFER_CHARACTERS_MAX);
            break;
    }
    char *cmd = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    snprintf(cmd, BUFFER_CHARACTERS_MAX, "SELECT %s FROM %s ORDER BY id DESC LIMIT %d", name, g_tablename, DATA_NB_ELMT_MAX);
    char *res = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    memset(res, '\0', BUFFER_CHARACTERS_MAX+1);
    size_t size_res = 0;
    mysql_command(con, cmd, res, &size_res);
    LV_LOG_USER("Raw [%ld] %s", size_res, res);

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
    free(name);
    free(cmd);
    free(res);
}

/* Get the lastest data */
void get_data_dashboard(MYSQL *con, int *data, int *nb_elmt) {
    assert((NULL!=con) && (NULL!=data) && (NULL!=nb_elmt));

    char *cmd = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    snprintf(cmd, BUFFER_CHARACTERS_MAX, "SELECT * FROM %s ORDER BY id DESC LIMIT 1", g_tablename);
    char *res = malloc(sizeof(char)*(BUFFER_CHARACTERS_MAX+1));
    memset(res, '\0', BUFFER_CHARACTERS_MAX+1);
    size_t size_res = 0;
    mysql_command(con, cmd, res, &size_res);
    LV_LOG_USER("Raw [%ld] %s", size_res, res);

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