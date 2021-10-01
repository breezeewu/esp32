#include <stdio.h>
#include "mqtt_command.h"
#include "sun_sdk_inc/sun_aiot_api.h"

aiot_mqtt_command* g_prmc_list = NULL;
int g_nmax_cmd_size = 100;
int init_command_list(int max_cmd_size)
{
    g_nmax_cmd_size = max_cmd_size;
    g_prmc_list = (aiot_mqtt_command*)malloc(sizeof(aiot_mqtt_command) * g_nmax_cmd_size);
    if(!g_prmc_list) return -1;
    memset(g_prmc_list, 0, sizeof(aiot_mqtt_command) * g_nmax_cmd_size);

    return 0;
}

int aiot_mqtt_add_bool_command(int cmd_id, const char* pword)
{
    int* pval_list = NULL;
    if(cmd_id < 0 || cmd_id >= g_nmax_cmd_size || NULL == pword)
    {
        lberror("Invalid parameter, cmd_id:%d, pword:%p\n", cmd_id, pword);
        return -1;
    }

    aiot_mqtt_add_int_command(cmd_id, pword, 0, 1);
    g_prmc_list[cmd_id].val_type = e_key_value_type_bool;

    return 0;
}

int aiot_mqtt_add_int_command(int cmd_id, const char* pword, int min, int max)
{
    int* pval_list = NULL;
    if(cmd_id < 0 || cmd_id >= g_nmax_cmd_size || NULL == pword)
    {
        lberror("Invalid parameter, cmd_id:%d, pword:%p\n", cmd_id, pword);
        return -1;
    }

    g_prmc_list[cmd_id].cmd_id = cmd_id;
    lbstrcp(g_prmc_list[cmd_id].pcmd_work, pword);
    g_prmc_list[cmd_id].val_type = e_key_value_type_int;
    pval_list = malloc(sizeof(int)*2);
    pval_list[0] = min;
    pval_list[0] = max;
    g_prmc_list[cmd_id].pvalue_list = pval_list;
    g_prmc_list[cmd_id].list_size = 2;

    return 0;
}

int aiot_mqtt_add_string_command(int cmd_id, const char* pword, char** ppval_list, int list_size)
{
    char* pstr_list = NULL;
    if(cmd_id < 0 || cmd_id >= g_nmax_cmd_size || NULL == pword)
    {
        lberror("Invalid parameter, cmd_id:%d, pword:%p\n", cmd_id, pword);
        return -1;
    }

    g_prmc_list[cmd_id].cmd_id = cmd_id;
    lbstrcp(g_prmc_list[cmd_id].pcmd_work, pword);
    g_prmc_list[cmd_id].val_type = e_key_value_type_int;
    pstr_list = (char*)malloc(sizeof(char*)*list_size);
    memset(pstr_list, 0, sizeof(char*));
    for(int i = 0; i < list_size; i++)
    {
        lbstrcp(pstr_list[i], ppval_list[i]);
    }
    g_prmc_list[cmd_id].list_size = list_size;

    return 0;
}

int aiot_mqtt_add_double_command(int cmd_id, const char* pword, double dmin, double dmax)
{
    double* pval_list = NULL;
    if(cmd_id < 0 || cmd_id >= g_nmax_cmd_size || NULL == pword)
    {
        lberror("Invalid parameter, cmd_id:%d, pword:%p\n", cmd_id, pword);
        return -1;
    }

    g_prmc_list[cmd_id].cmd_id = cmd_id;
    lbstrcp(g_prmc_list[cmd_id].pcmd_work, pword);
    g_prmc_list[cmd_id].val_type = e_key_value_type_double;
    pval_list = malloc(sizeof(double)*2);
    pval_list[0] = dmin;
    pval_list[0] = dmax;
    g_prmc_list[cmd_id].pvalue_list = pval_list;
    g_prmc_list[cmd_id].list_size = 2;

    return 0;
}

int aiot_mqtt_sample_send_command(void* handle, int cmd_id, int index)
{
    char cmd_state[100];
    sprintf(cmd_state, "{\"%s\": \"%s\"}", g_prmc_list[cmd_id].pcmd_work, g_prmc_list[cmd_id].pvalue_list);
    sun_aiot_api_mqtt_pub_property(handle, )
}