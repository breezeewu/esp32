#ifndef _AIOT_UTIL_H_
#define _AIOT_UTIL_H_
#include "sun_aiot_api.h"
//#include "cjson.h"
#define MAX_RESPONSE_SIZE           1024
//#define SUN_DEFAULT_MQTT_HOST_FORMAT
#define SUN_AIOT_API_DEVICE_DEFAULT_URL_FORMAT                      "https://iot-api%s.sunvalleycloud.com%s"
#define SUN_AIOT_API_DEVICE_OTA_URL_FORMAT                          "https://open-api%s.sunvalleycloud.com%s"
#define SUN_AIOT_API_DEVICE_PARAM_FORMAT                            "?access_token=%s&timestamp=%lu&lang=%s"
#define SUN_AIOT_OTA_PATH                                           "/api/ota/upgrade/firmware"
typedef struct{
    int        stateCode;
    char*      stateMsg;
    char*      response;
    int        resp_len;

    struct cJSON*     json_data;
    struct cJSON*     json_root;
    struct cJSON*     json_obj;
   
} http_response_context;

typedef struct{
    unsigned long   lupdate_time;
    char*           access_token;
    char*           token_type;
    char*           refresh_token;
    int             expires_in;
    char*           scope;
} sun_aiot_token_context;

typedef struct {
    char*                   client_id;
    char*                   client_secret;
    char*                   device_sn;
    char*                   crt_path;
    char*                   host_format;
    char*                   plang;
    e_aiot_env_type         env_type;
    struct lbmutex*         pmutex;
    void*                   mqtt_handle;
    int                     mqtt_keep_live_time;

    char*                   pmqtt_host_format;
    char*                   paiot_url_format;
    int*                    mqtt_port_list[5];
    // token
    struct lbthread_context*  ptoken_thread;
    sun_aiot_token_context  atc;
} sun_aiot_context;

typedef struct
{
    int     nminPower;
    int     nfileSize;
    int     nflow;
    int     nmethod;
    int     nstateCode;

    char*   pfileUrl;
    char*   pimei;
    char*   pmd5;
    char*   pofferId;
    char*   ppublishTime;
    char*   psn;
    char*   phints;
    char*   pversion;
    char*   pstartTime;
    char*   pendTime;
    char*   pstateMsg;
} ota_resp_info;

e_aiot_env_type get_env_by_device_sn(const char* device_sn);

int load_config_from_http(sun_aiot_context* psac, const char* purl);

int load_config_from_file(sun_aiot_context* psac, const char* path);

int get_http_url_from_path(sun_aiot_context* psac, const char* format, const char* path, char* http_url, int url_size);

http_response_context* lbhttp_response_context_open(char* resp, int resp_len);

void lbhttp_response_context_close(http_response_context** pphrc);

int http_send_and_get_response(sun_aiot_context* psac, const char* format, const char* path, const char* req, int* phttp_code, char* resp, int resp_len);

int http_post(sun_aiot_context* psac, const char* format, const char* path, const char* req, int* pstatus_code, http_response_context** pphrc);

int esp_http_send_and_get_response(sun_aiot_context* psac, const char* format, const char* path, const char* req, int* phttp_code, char* resp, int size);

int esp_http_post(sun_aiot_context* psac, const char* format, const char* path, const char* req, http_response_context** pphrc);
// mqtt config
const char* get_mqtt_host_name(e_aiot_env_type env_type, int* pport);

int fetch_string_from_json(struct cJSON* pjn, const char* pkey, char** ppval);

int fetch_int_from_json(struct cJSON* pjn, const char* pkey, int* pnval);

int fetch_double_from_json(struct cJSON* pjn, const char* pkey, double* pdval);

//void* http_create_download_task(sun_aiot_context* psac, const char* purl, const char* dst_path, on_download_cb download_cb, void* owner);

int http_download_start_task(void* task_id);
//void* http_download_start_task(const char* purl, int* phttp_code, const char* dst_path, on_download_cb download_cb, void* owner);

int http_download_get_progress(void* task_id, e_aiot_http_msg_type* msg_type, http_download_info* phdp, int* percent);

//int http_download_get_progress(void* task_id, long* speed, long* download_bytes, long* total_bytes, long* spend_time_ms);

int http_download_stop_task(void* task_id);

// gen url from aiot context
int gen_mqtt_host_and_port(sun_aiot_context* psac, char* host, int size, int* pport);

int gen_http_url_from_path(sun_aiot_context* psac, const char* path, char* purl, int size);

ota_resp_info* http_post_ota_request(sun_aiot_context* psac, aiot_ota_info* pota_info);

void http_free_ota_response(ota_resp_info** ppori);
//int http_post_ota_request(sun_aiot_context* psac, aiot_ota_info* pota_info);

#endif