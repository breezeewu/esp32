#pragma once
typedef enum
{
    e_key_value_type_bool     = 0,
    e_key_value_type_int     = 1,
    //e_key_value_type_range    = 2,
    e_key_value_type_string   = 3,
    e_key_value_type_double   = 4
} e_key_value_type;

typedef struct{
    int                 cmd_id;
    char*               pcmd_work;
    e_key_value_type    val_type;
    int                 list_size;
    /*union value{
        char*               pstr_value_list;
        long*               plong_value_list;
        double*             pdoulbe_value_list;
    };*/
    //char* pstr_value_list;
    
    void* pvalue_list;
} aiot_mqtt_command;

extern aiot_mqtt_command* g_prmc_list;

int init_command_list(int max_cmd_size);

int aiot_mqtt_add_bool_command(int cmd_id, const char* word);

int aiot_mqtt_add_int_command(int cmd_id, const char* word, int min, int max);

int aiot_mqtt_add_string_command(int cmd_id, const char* word, char** ppval_list, int list_size);

int aiot_mqtt_add_double_command(int cmd_id, const char* word, double dmin, double dmax);

int aiot_mqtt_sample_send_command(void* handle, int cmd_id, int index);