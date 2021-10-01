#include <pthread.h>
#include "lbmacro.h"
#include "sun_sdk_inc/sun_aiot_api.h"
#include "sun_mqtt_api.h"
#include "cJSON.h"
#include "lbthread.h"
#include "aiot_util.h"
#include "lbhttp.h"
//#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota.h"

//#include "esp_ota_ops.h"
//#define ENABLE_WIFI
#ifdef ENABLE_WIFI
#include "mywifi.h"
#endif
#include "lbmbedtls.h"
#include "lbstring.h"
//#define ENABLE_ESP_HTTP
#define SUN_AIOT_API_URL_FORMAT     "https://iot-api%s.sunvalleycloud.com%s"
#define SUN_AIOT_API_HOST_DEV        "https://iot-api-dev.sunvalleycloud.com"
#define SUN_AIOT_API_HOST_TEST       "https://iot-api-test.sunvalleycloud.com"
#define SUN_AIOT_API_HOST_SIT        "https://iot-api-sit.sunvalleycloud.com"
#define SUN_AIOT_API_HOST_DEMO       "https://iot-api-demo.sunvalleycloud.com"
#define SUN_AIOT_API_HOST_PROB       "https://iot-api.sunvalleycloud.com"
#define SUN_AIOT_API_PATH            "/oauth/login"
#define SUN_AIOT_REFRESH_TOKEN_PATH  "/oauth/refresh-token"
#define MAX_RESPONSE_SIZE           4096

#define SUN_AIOT_DEFAULT_MQTT_HOST_FORMAT           "iot-mqtt-server%s.sunvalleycloud.com"
#define SUN_AIOT_DEFAULT_URL_WITH_PARAM_FORMAT      "https://iot-api%s.sunvalleycloud.com%s?access_token=%s&timestamp=%lu&lang=en"

#define WIFI_SSID       "Tenda"
#define WIFI_PWD        "1234456789"
int sun_default_mqtt_port_list[5] = {9911, 9913, 9912, 9914, 9915};

int sun_aiot_update_token(sun_aiot_context* psac)
{
    /*lbinfo("sun_aiot_update_token 111\n");
    lbnet_mbed_tls_conenct_test(NULL, "iot-api-sit.sunvalleycloud.com", 443);
    psac->atc.lupdate_time = lbget_sys_time();
    psac->atc.expires_in = 0;
    return 0;*/
    cJSON* pjr = NULL;
    char* pstr = NULL;
    const char* path = NULL;
    //int status_code = 0;
    int ret = -1;
    unsigned long begin_time = lbget_sys_time();
    http_response_context* phrc = NULL;
    lbcheck_pointer(psac, -1, "Invalid parameter psac:%p\n", psac);
    //lbinfo("%s(psac:%p)\n", __FUNCTION__, psac);
    if( NULL == psac->client_id || NULL == psac->client_secret || NULL == psac->device_sn)
    {
        lberror("aiot context not init, psac->client_id:%s, psac->client_secre:%s, psac->device_sn:%s\n", psac->client_id, psac->client_secret, psac->device_sn);
        return -1;
    }

    pjr = cJSON_CreateObject();
    lbmem_add_ref(pjr, sizeof(cJSON));
    lbcheck_pointer(pjr, -1, "pjr:%p = cJSON_CreateObject()", pjr);
    //lbinfo("lbmutex_lock(psac->pmutex) end\n");
    lbmutex_lock(psac->pmutex);
    //lbinfo("%s(psac:%p) begin\n", __FUNCTION__, psac);
    do{
        cJSON_AddStringToObject(pjr, "client_id", psac->client_id);
        cJSON_AddStringToObject(pjr, "client_secret", psac->client_secret);
        cJSON_AddStringToObject(pjr, "scope", "all");
        if(NULL == psac->atc.access_token || NULL == psac->atc.refresh_token)
        {
            cJSON_AddStringToObject(pjr, "grant_type", "password");
            cJSON_AddStringToObject(pjr, "sn", psac->device_sn);
            cJSON_AddStringToObject(pjr, "auth_type", "sn_password");
            path = SUN_AIOT_API_PATH;
        }
        else
        {
            cJSON_AddStringToObject(pjr, "grant_type", "refresh_token");
            cJSON_AddStringToObject(pjr, "refresh_token", psac->atc.refresh_token);
            path = SUN_AIOT_REFRESH_TOKEN_PATH;
        }

        pstr = cJSON_PrintUnformatted(pjr);
        lbmem_add_ref(pstr, strlen(pstr));
        cJSON_Delete(pjr);
        lbmem_rm_ref(pjr);
        pjr = NULL;
		lbfree(psac->atc.access_token);
#ifdef ENABLE_ESP_HTTP
        ret = esp_http_post(psac, NULL, path, pstr, &phrc);
#else
        ret = http_post(psac, NULL, path, pstr, NULL, &phrc);
#endif
        //ret = http_post(psac, NULL, path, pstr, &status_code, &phrc);
        lbinfo("ret:%d = http_post(psac:%p, path:%s, pstr:%p, &phrc:%p)\n", ret, psac, path, pstr, phrc);
        if(ret < 0 || NULL == phrc || NULL == phrc->json_data)
        {
            lberror("ret:%d = http_post(psac:%p, SUN_AIOT_API_PATH:%s, pstr:%s, &phrc:%p) failed, phrc->json_data：%p\n", ret, psac, path, pstr, phrc, phrc ? phrc->json_data : NULL);
            break;
        }

        lbcheck_pointer(phrc->json_data, -1, "http_post failed, phrc->json_data:%p\n", phrc->json_data);
        ret = fetch_string_from_json(phrc->json_data, "access_token", &psac->atc.access_token);
        lbcheck_break(ret, "fetch access_token from response failed");

        ret = fetch_string_from_json(phrc->json_data, "refresh_token", &psac->atc.refresh_token);
        lbcheck_break(ret, "fetch refresh_token from response failed");

        ret = fetch_string_from_json(phrc->json_data, "token_type", &psac->atc.token_type);
        lbcheck_break(ret, "fetch token_type from response failed");

        ret = fetch_int_from_json(phrc->json_data, "expires_in", &psac->atc.expires_in);
        lbcheck_break(ret, "fetch expires_in from response failed");

        ret = fetch_string_from_json(phrc->json_data, "scope", &psac->atc.scope);
        lbcheck_break(ret, "fetch scope from response failed");
        psac->atc.lupdate_time = lbget_sys_time();
        lbhttp_response_context_close(&phrc);

        lbtrace("token update success, access_token:%s, refresh_token:%s, token_type:%s, expires_in:%d, scope:%s, spend time:%lu\n", psac->atc.access_token, psac->atc.refresh_token, psac->atc.token_type, psac->atc.expires_in, psac->atc.scope, lbget_sys_time() - begin_time);
    }while(0);
    lbinfo("after while\n");
    lbfree(pstr);
    //psac->atc.expires_in = 5;
    lbmutex_unlock(psac->pmutex);
    return ret;
}

void* token_refresh_proc(void* arg)
{
    int ret = 0;
    sun_aiot_context* psac = NULL;
    struct lbthread_context* ptc = (struct lbthread_context*)arg;
    lbcheck_pointer(ptc, NULL, "Invalid parameter ptc:%p\n", ptc);
    psac = (sun_aiot_context*)lbthread_get_owner(ptc);
    lbcheck_pointer(psac, NULL, "psac:%p = (sun_aiot_context*)lbthread_get_owner(ptc) failed\n", psac);

    while(lbthread_is_alive(ptc))
    {
        if(lbget_sys_time() - psac->atc.lupdate_time >= psac->atc.expires_in * 1000)
        {
            ret = sun_aiot_update_token(psac);
            if(ret != 0)
            {
                lberror("ret:%d = sun_aiot_update_token(psac:%p) failed\n", ret, psac);
                sleep(1);
            }
        }

        usleep(20000);
    }
    return NULL;
}

#ifdef ENABLE_WIFI
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    printf("ev_handle_called.\n");
    switch(event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI("wifi", "WIFI_EVENT_STA_START");
    //do not actually connect in test case
            //;
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI("wifi", "WIFI_EVENT_STA_DISCONNECTED");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        default:
            break;
    }
    return ;
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    ip_event_got_ip_t *event;
    printf("ev_handle_called.\n");
    switch(event_id) {
        case IP_EVENT_STA_GOT_IP:
            event = (ip_event_got_ip_t*)event_data;
            ESP_LOGI("wifi", "IP_EVENT_STA_GOT_IP");
            ESP_LOGI("wifi", "got ip:" IPSTR "\n", IP2STR(&event->ip_info.ip));
            break;
        default:
            break;
    }

    return ;
}

static int event_init(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));
    return ESP_OK;
}

int wifi_start()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //esp_err_t r = nvs_flash_init();
    //if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //    printf("no free pages or nvs version mismatch, erase..\n");
    //    TEST_ESP_OK(nvs_flash_erase());
    //    r = nvs_flash_init();
    //}
    //TEST_ESP_OK( r);
    esp_netif_init();
    event_init();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PWD
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    return 0;
}
#endif

void* sun_aiot_api_init(const char* client_id, const char* client_secret, const char* device_sn, const char* pcrt_path, e_aiot_env_type env_type)
{
    sun_aiot_context* psac = NULL;
    //lbinfo("sun_aiot_api_init client_id:%s, client_secret:%s, device_sn:%s, pcrt_path:%s\n", client_id, client_secret, device_sn, pcrt_path);
    if(NULL == client_id || NULL == client_secret || NULL == device_sn)
    {
        lberror("Invalid parameter, client_id:%s, client_secret:%s, device_sn:%s\n", client_id, client_secret, device_sn);
        return 0;
    }
    
    
    psac = (sun_aiot_context*)lbmalloc(sizeof(sun_aiot_context));
    memset(psac, 0, sizeof(sun_aiot_context));

    psac->mqtt_keep_live_time = 30;
    lbstrcp(psac->client_id, client_id);
    lbstrcp(psac->client_secret, client_secret);
    lbstrcp(psac->device_sn, device_sn);
    lbstrcp(psac->crt_path , pcrt_path);
    lbstrcp(psac->pmqtt_host_format, SUN_AIOT_DEFAULT_MQTT_HOST_FORMAT);
    lbstrcp(psac->paiot_url_format, SUN_AIOT_DEFAULT_URL_WITH_PARAM_FORMAT);
    lbstrcp(psac->plang, "en");
    memcpy(&psac->mqtt_port_list, &sun_default_mqtt_port_list, sizeof(sun_default_mqtt_port_list));

    psac->env_type = env_type == e_aiot_env_default ? get_env_by_device_sn(device_sn) : env_type;
    //printf("before lbmalloc mutex\n");
    psac->pmutex = (lbmutex*)lbmalloc(sizeof(lbmutex));
    lbmutex_recursive_init(psac->pmutex);

    psac->mqtt_handle = sun_mqtt_init(psac->device_sn, psac->client_id, psac->client_secret, psac->crt_path);
    lbcheck_pointer(psac->mqtt_handle, NULL, "psac->mqtt_handle:%p = sun_mqtt_init(psac->device_sn:%s, psac->client_id:%s, psac->client_secret:%s, psac->crt_path:%s)\n", psac->mqtt_handle, psac->device_sn, psac->client_id, psac->client_secret, psac->crt_path);
    //lbinfo("psac->mqtt_handle:%p = sun_mqtt_init(psac->device_sn:%s, psac->client_id:%s, psac->client_secret:%s, psac->crt_path:%s)\n", psac->mqtt_handle, psac->device_sn, psac->client_id, psac->client_secret, psac->crt_path);
    //printf("after lbmalloc mutex\n");
    lbtrace("psac:%p = %s(client_id:%s, client_secret:%s, device_sn:%s, pcrt_path:%s, psac->env_type:%d)\n", psac, __FUNCTION__, client_id, client_secret, device_sn, pcrt_path, psac->env_type);
    //printf("sun_aiot_api_init end\n");
#ifdef ENABLE_WIFI
    lbconnect_wifi(WIFI_SSID, WIFI_PWD);
#endif
    //printf("connect wifi end\n");
    return psac;
}

void sun_aiot_api_deinit(void** phandle)
{
    lbtrace("%s(phandle:%p) begin\n", __FUNCTION__, phandle);
    if(phandle && *phandle)
    {
        sun_aiot_context* psac = (sun_aiot_context*)*phandle;

        if(psac)
        {
            lbmutex_lock(psac->pmutex);
            // aiot token context
            lbfree(psac->atc.access_token);
            lbfree(psac->atc.token_type);
            lbfree(psac->atc.refresh_token);
            lbfree(psac->atc.scope);

            // aiot context
            lbfree(psac->client_id);
            lbfree(psac->client_secret);
            lbfree(psac->device_sn);
            lbfree(psac->crt_path);
            lbfree(psac->host_format);
            lbfree(psac->pmqtt_host_format);
            lbfree(psac->paiot_url_format);
            if(psac->mqtt_handle)
            {
                sun_mqtt_deinit(&psac->mqtt_handle);
            }
            if(psac->ptoken_thread)
            {
                lbthread_stop(psac->ptoken_thread);
                lbthread_close(psac->ptoken_thread);
            }
            lbmutex_unlock(psac->pmutex);
            lbmutex_deinit(psac->pmutex);
            lbfree(psac);
        }
    }
    lbtrace("%s end\n", __FUNCTION__);

#ifdef LB_MEMORY_CHECK_H_
    lbmem_check_deinit();
#endif
}

int sun_aiot_api_load_config(void* handle, const char* purl)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == purl)
    {
        lberror("Invalid parameter, psac:%p, purl:%s\n", psac, purl);
        return -1;
    }

    if(memcmp("http", purl, 4) == 0)
    {
        return load_config_from_http(psac, purl);
    }
    else
    {
        return load_config_from_file(psac, purl);
    }
}

int sun_aiot_api_init_log(int level, int mode, const char* path)
{
    printf("sun_aiot_api_init_log(level:%d, mode:%d, path:%s)\n", level, mode, path);
    return lbinit_log(path, level, mode, SUN_AIOT_COMMON_SDK_VERSION_MAJOR, SUN_AIOT_COMMON_SDK_VERSION_MINOR, SUN_AIOT_COMMON_SDK_VERSION_MACRO, SUN_AIOT_COMMON_SDK_VERSION_TINY);
    //log_ctx* g_plogctx = lbmalloc();
}

int sun_aiot_api_connect(void* handle)
{
    int ret = -1;
    int port = 0;
    char mqtt_host[256];
    http_response_context* phrc = NULL;
    sun_aiot_context* psac = (sun_aiot_context*)handle;

    if(NULL == psac || NULL == psac->client_id || NULL == psac->client_secret || NULL == psac->device_sn)
    {
        lberror("Invalid parameter, psac:%p\n", psac);
        return -1;
    }

    lbmutex_lock(psac->pmutex);
    lbinfo("before while\n");
    do{
        /*ret = sun_aiot_update_token(psac);
        lbinfo("ret:%d = sun_aiot_update_token(psac:%p)\n", ret, psac);
        lbcheck_break(ret, "ret:%d = sun_aiot_login(psac:%p)\n", ret, psac);*/

        // create mqtt connect
        if(NULL == psac->mqtt_handle)
        {
            psac->mqtt_handle = sun_mqtt_init(psac->device_sn, psac->client_id, psac->client_secret, psac->crt_path);
            lbcheck_pointer_break(psac->mqtt_handle, "psac->mqtt_handle:%p = sun_mqtt_init(psac->device_sn:%s, psac->client_id:%s, psac->client_secret:%s, psac->crt_path:%s)\n", psac->mqtt_handle, psac->device_sn, psac->client_id, psac->client_secret, psac->crt_path);
            lbinfo("psac->mqtt_handle:%p = sun_mqtt_init(psac->device_sn:%s, psac->client_id:%s, psac->client_secret:%s, psac->crt_path:%s)\n", psac->mqtt_handle, psac->device_sn, psac->client_id, psac->client_secret, psac->crt_path);
        }

        gen_mqtt_host_and_port(psac, mqtt_host, 256, &port);
        lbinfo("gen_mqtt_host_and_port(psac, mqtt_host:%s, 256, &port:%d)\n", mqtt_host, port);
        if(strlen(mqtt_host) <= 0 || port <= 0)
        {
            lberror("%s get_mqtt_host_name failed, mqtt_host_name:%s, port:%d\n", __FUNCTION__, mqtt_host, port);
            break;
        }

        ret = sun_mqtt_connect(psac->mqtt_handle, mqtt_host, port, psac->mqtt_keep_live_time);
        lbinfo("ret:%d = sun_mqtt_connect(psac->mqtt_handle:%p, mqtt_host:%s, port:%d, psac->mqtt_keep_live_time:%d)\n", ret, psac->mqtt_handle, mqtt_host, port, psac->mqtt_keep_live_time);
        lbcheck_break(ret, "ret:%d = sun_mqtt_connect(psac->mqtt_handle:%p, mqtt_host:%s, port:%d, psac->mqtt_keep_live_time:%d)\n", ret, psac->mqtt_handle, mqtt_host, port, psac->mqtt_keep_live_time);
        //lbinfo("ret:%d = sun_mqtt_connect(psac->mqtt_handle:%p, pmqtt_host:%s, port:%d, psac->mqtt_keep_live_time:%d)\n", ret, psac->mqtt_handle, pmqtt_host, port, psac->mqtt_keep_live_time);

        psac->ptoken_thread = lbthread_open("reflresh token", psac, token_refresh_proc);
        lbcheck_pointer_break(psac->ptoken_thread, "psac->ptoken_thread:%p = lbthread_open(psac:%p, token_refresh_proc:%p) failed\n", psac->ptoken_thread, psac, token_refresh_proc);
        ret = lbthread_start(psac->ptoken_thread);
        lbcheck_break(ret, "ret:%d = lbthread_start(psac->ptoken_thread:%p)\n", ret, psac->ptoken_thread);
        lbinfo("ret:%d = lbthread_start(psac->ptoken_thread:%p)\n", ret, psac->ptoken_thread);
    }while(0);
    //lbinfo("while(0)\n");
    lbmutex_unlock(psac->pmutex);
    //lbinfo("lbmutex_unlock\n");
    lbhttp_response_context_close(&phrc);
    lbtrace("%s end ret:%d\n", __FUNCTION__, ret);
    return ret;
}

void sun_aiot_api_disconnect(void* handle)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac)
    {
        lberror("Invalid parameter, psac:%p\n", psac);
        return ;
    }
    lbtrace("%s(psac:%p)\n", __FUNCTION__, psac);
    if(psac->ptoken_thread)
    {
        lbthread_stop(psac->ptoken_thread);
        lbthread_close(&psac->ptoken_thread);
    }
    //lbinfo("%s destroy mqtt_handle:%p\n", __FUNCTION__, psac->mqtt_handle);
    if(psac->mqtt_handle)
    {
        sun_mqtt_disconnect(psac->mqtt_handle);
        sun_mqtt_deinit(&psac->mqtt_handle);
    }
    lbtrace("%s end\n", __FUNCTION__);
}

int sun_aiot_api_http_post(void* handle, const char* path, const char* req, int* phttp_code, char* resp, int len)
{
    int ret = 0;
    sun_aiot_context* psac = (sun_aiot_context*)handle;

    if(NULL == psac)
    {
        lberror("Invalid parameter, psac:%p\n", psac);
        return -1;
    }
#ifdef ENABLE_ESP_HTTP
    ret = esp_http_send_and_get_response(psac, NULL, path, req, phttp_code, resp, len);
#else
    ret = http_send_and_get_response(psac, NULL, path, req, phttp_code, resp, len);
#endif
    lbtrace("ret:%d = http_send_and_get_response(psac:%p, path:%s, req:%s, *phttp_code:%d, resp:%s, len:%d)\n", ret, psac, path, req, phttp_code ? *phttp_code : 0, resp, len);
    return ret;
}

void* sun_aiot_api_create_oat_download_task(void* handle, const char* dst_path, on_http_msg_cb download_cb, void* owner, aiot_ota_info* pota_info)
{
    void* task_id = NULL;
    ota_resp_info* pori = NULL;
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    lbcheck_pointer(psac, NULL, "Invalid parameter, psac:%p\n", psac);
    lbinfo("%s(handle:%p, dst_path:%p, download_cb:%p, owner:%p, pota_info:%p)", __FUNCTION__, handle, dst_path, download_cb, owner, pota_info);
    pori = http_post_ota_request(psac, pota_info);
    lbcheck_pointer(pori, NULL, "pori:%p = http_post_ota_request(psac:%p, pota_info:%p)", pori, psac, pota_info);
    
    if(pori->nminPower > pota_info->state_of_charge || lbstring_version_compare(pota_info->version, pori->pversion) >= 0)
    {
        lberror("create ota download task failed, pori->nminPower:%d > pota_info->state_of_charge:%d || lbstring_version_compare(pota_info->version, pori->pversion):%d\n", pori->nminPower, pota_info->state_of_charge, lbstring_version_compare(pota_info->version, pori->pversion));
        http_free_ota_response(pori);
        return NULL;
    }
    printf("get ota url, pori->pfileUrl:%s\n", pori->pfileUrl);
    char* ota_path = "https://live-play-sit.sunvalleycloud.com/ota/hello-world.bin";
    task_id = esp_http_create_ota_task(ota_path);
    lbinfo("task_id:%p = esp_http_create_ota_task(ota_path:%s)\n", task_id, ota_path);
    /*esp_http_ota_start_task(task_id);
    sun_aiot_api_create_download_task(handle, )
    task_id = http_create_download_task(psac, pori->pfileUrl, dst_path, download_cb, owner);
    lbtrace("task_id:%p = http_create_download_task(psac:%p, pori->pfileUrl:%s, dst_path:%p, download_cb:%p, owner:%p)\n", task_id, psac, pori->pfileUrl, dst_path, download_cb, owner);
	http_free_ota_response(pori);*/
    return task_id;
}

/*void* sun_aiot_api_create_download_task(void* handle, const char* path, const char* dst_path, on_http_msg_cb download_cb, void* owner)
{
    void* task_id = NULL;
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    lbcheck_pointer(psac, NULL, "Invalid parameter, psac:%p\n", psac);
    task_id = http_create_download_task(psac, path, dst_path, download_cb, owner);
    lbtrace("task_id:%p = http_create_download_task(psac:%p, path:%s, dst_path:%p, download_cb:%p, owner:%p)\n", task_id, psac, path, dst_path, download_cb, owner);

	return task_id;
}*/
 
/*void* sun_aiot_api_http_download_start_task(void* handle, const char* path, int* phttp_code, const char* dst_path, on_http_msg_cb download_cb, void* owner)
{
    char url = lbmalloc(MAX_URL_SIZE);
    void* task_id = NULL;
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    int ret = get_http_url_from_path(psac, path, url, MAX_URL_SIZE);
    if(ret <= 0)
    {
        lberror("ret:%d = get_http_url_from_path(psac->env_type:%d, path:%s, url, MAX_URL_SIZE)\n", ret, psac->env_type, path);
        return NULL;
    }
    lbinfo("sun_aiot_api_http_download_start_task begin\n");
    //lbcheck_return(ret, "ret:%d = get_http_url_from_path(psac->env_type:%d, path:%s, url, 1024)\n", ret, psac->env_type, path);
    task_id = http_download_start_task(url, phttp_code, dst_path, download_cb, owner);
    lbcheck_pointer(task_id, NULL, "ret:%d = http_download_start_task(url:%s, phttp_code:%d, dst_path:%s, download_cb:%p, owner:%p)\n", ret, url, phttp_code ? *phttp_code : 0, dst_path, download_cb, owner);
    return task_id;
}*/

int sun_aiot_api_http_download_start(void* task_id)
{
    //int ret =  http_download_start_task(task_id);
    //lbtrace("ret:%d =  http_download_start_task(task_id:%p)\n", ret, task_id);
    int ret = esp_http_ota_start_task(task_id);
    lbinfo("ret:%d = esp_http_ota_start_task(task_id:%p)\n", ret, task_id);
    return ret;
}

int sun_aiot_api_http_download_get_progress(void* task_id, e_aiot_http_msg_type* msg_type, http_download_info* phdi, int* percent)
{
    //int ret = http_download_get_progress(task_id, msg_type, phdp,  percent);
    //lbinfo("ret:%d = http_download_get_progress(task_id, msg_type, phdp,  percent:%d)\n", ret, percent ? -1 : *percent);
    
    int ret = esp_http_ota_get_progress(task_id, msg_type, phdi, percent);
    lbinfo("ret:%d = esp_http_ota_get_progress(task_id:%p, msg_type:%p, phdi:%p, percent:%p)\n", ret, task_id, msg_type, phdi, percent);
    return ret;
}

int sun_aiot_api_http_download_stop(void* task_id)
{
    //int ret = http_download_stop_task(task_id);
    //lbtrace("ret:%d =  http_download_stop_task(task_id:%p)\n", ret, task_id);
    int ret = esp_http_ota_stop_task(task_id);
    lbinfo("ret:%d = esp_http_ota_stop_task(task_id:%p)\n", ret, task_id);
    return ret;
}

int sun_aiot_api_version(char* szver, int len)
{
    char VersionId[50] = {0};
    const char* fmt = "{\"Owner\":\"%s\" ,\"VersionId\":\"%s\", \"VersionDate\":\"%s\", \"VersionTime\":\"%s\"}";
    const char* Owner = "Sunvellay All Rights Reserved";
    int need_len = 0;
    if (NULL == szver) {
        lberror("szver == NULL!\n");
        return -1;
    }
    need_len = strlen(VersionId) + strlen(fmt) + strlen(Owner) + strlen(__DATE__)+ strlen(__TIME__) + 2;
    sprintf(VersionId,"%d.%d.%d.%d", SUN_AIOT_COMMON_SDK_VERSION_MAJOR, SUN_AIOT_COMMON_SDK_VERSION_MINOR, SUN_AIOT_COMMON_SDK_VERSION_MACRO, SUN_AIOT_COMMON_SDK_VERSION_TINY);
    if(need_len > len)
    {
        lberror("not enought bufer to receive version info, need:%d, have:%d\n", need_len, len);
        return -1;
    }
    memset(szver, 0, len);
    sprintf(szver, fmt, Owner, VersionId, __DATE__, __TIME__);
    lbtrace(szver);

    return 0;
}

int sun_aiot_api_mqtt_register_event_handle(void* handle, mqtt_event_callback mqtt_event_cb, void* powner)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == psac->mqtt_handle)
    {
        lberror("Invalid parameter, psac:%p, psac->mqtt_handle:%p\n", psac, psac ? psac->mqtt_handle : NULL);
        return -1;
    }

    return sun_mqtt_register_event_handle(psac->mqtt_handle, mqtt_event_cb, powner);
}

int sun_aiot_api_mqtt_pub(void* handle, const char* ptopic, const char* payload, int payload_len, e_aiot_mqtt_qos qos)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == psac->mqtt_handle)
    {
        lberror("Invalid parameter, psac:%p, psac->mqtt_handle:%p\n", psac, psac ? psac->mqtt_handle : NULL);
        return -1;
    }

    return sun_mqtt_pub(psac->mqtt_handle, ptopic, payload, payload_len, qos);
}

int sun_aiot_api_mqtt_quick_pub(void* handle, e_aiot_mqtt_msg_type msg_type, const char* event, const char* command, const char* data, const char* state, const char* result, e_aiot_mqtt_qos qos)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == psac->mqtt_handle)
    {
        lberror("Invalid parameter, psac:%p, psac->mqtt_handle:%p\n", psac, psac ? psac->mqtt_handle : NULL);
        return -1;
    }

    return sun_mqtt_quick_pub(psac->mqtt_handle, msg_type, event, command, data, state, result, qos);
}

int sun_aiot_api_mqtt_pub_ctrl(void* handle, const char* command, e_aiot_mqtt_qos qos)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == psac->mqtt_handle)
    {
        lberror("Invalid parameter, psac:%p, psac->mqtt_handle:%p\n", psac, psac ? psac->mqtt_handle : NULL);
        return -1;
    }

    return sun_mqtt_pub_ctrl(psac->mqtt_handle, command, qos);
}

int sun_aiot_api_mqtt_pub_update(void* handle, const char* state, const char* result, e_aiot_mqtt_qos qos)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == psac->mqtt_handle)
    {
        lberror("Invalid parameter, psac:%p, psac->mqtt_handle:%p\n", psac, psac ? psac->mqtt_handle : NULL);
        return -1;
    }

    return sun_mqtt_pub_update(psac->mqtt_handle, state, result, qos);
}

int sun_aiot_api_mqtt_pub_event(void* handle, const char* event, const char* data, e_aiot_mqtt_qos qos)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == psac->mqtt_handle)
    {
        lberror("Invalid parameter, psac:%p, psac->mqtt_handle:%p\n", psac, psac ? psac->mqtt_handle : NULL);
        return -1;
    }

    return sun_mqtt_pub_event(psac->mqtt_handle, event, data, qos);
}

int sun_aiot_api_mqtt_pub_property(void* handle, const char* state, e_aiot_mqtt_qos qos)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == psac->mqtt_handle)
    {
        lberror("Invalid parameter, psac:%p, psac->mqtt_handle:%p\n", psac, psac ? psac->mqtt_handle : NULL);
        return -1;
    }

    return sun_mqtt_pub_property(psac->mqtt_handle, state, qos);
}

int sun_aiot_api_mqtt_sub(void* handle, const char* ptopic, e_aiot_mqtt_qos qos, on_mqtt_subscribe_cb on_topic_msg, void* userdata)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == psac->mqtt_handle)
    {
        lberror("Invalid parameter, psac:%p, psac->mqtt_handle:%p\n", psac, psac ? psac->mqtt_handle : NULL);
        return -1;
    }
    return sun_mqtt_sub(psac->mqtt_handle, ptopic, qos, on_topic_msg, userdata);
}

int sun_aiot_api_mqtt_unsub(void* handle, const char* ptopic)
{
    sun_aiot_context* psac = (sun_aiot_context*)handle;
    if(NULL == psac || NULL == psac->mqtt_handle)
    {
        lberror("Invalid parameter, psac:%p, psac->mqtt_handle:%p\n", psac, psac ? psac->mqtt_handle : NULL);
        return -1;
    }
    lbinfo("%s begin\n", __FUNCTION__);
    return sun_mqtt_unsub(psac->mqtt_handle, ptopic);
}
