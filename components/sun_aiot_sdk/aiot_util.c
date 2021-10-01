#include "lbmacro.h"
#include "sun_sdk_inc/sun_aiot_api.h"
#include "aiot_util.h"
#include "lbhttp.h"
#include "cJSON.h"
#include "lbmbedtls.h"
#include "esp_http.h"
#define SUN_AIOT_DEVICE_API_URL_WITH_PARAM_FORMAT     "https://iot-api%s.sunvalleycloud.com%s?access_token=%s&timestamp=%lu&lang=%s"
#define SUN_AIOT_DEVICE_OTA_URL_FORMAT                  "https://open-api%s.sunvalleycloud.com%s?access_token=%s&timestamp=%lu&lang=%s"
#define MAX_HOST_TAG_SIZE           6
//int sun_default_mqtt_port_list[5] = {9911, 9913, 9912, 9914, 9915};

log_ctx* g_plogctx = NULL;

enum e_aiot_env_type get_env_by_device_sn(const char* device_sn)
{
    /*if(0 == memcmp(device_sn, "64X", 3))
    {
        return e_aiot_env_pro;
    }
    else */
    if(0 == memcmp(device_sn + 7, "0001", 4))
    {
        return e_aiot_env_dev;
    }
    else if(0 == memcmp(device_sn + 7, "0002", 4))
    {
        return e_aiot_env_sit;
    }
    else if(0 == memcmp(device_sn + 7, "0003", 4))
    {
        return e_aiot_env_test;
    }
    else
    {
        return e_aiot_env_pro;
    }
    

    //return e_aiot_env_unknown;
}
const char* get_host_tag_from(enum e_aiot_env_type env)
{
    switch(env)
    {
        case e_aiot_env_dev:
            return "-dev";
        case e_aiot_env_sit:
            return "-sit";
        case e_aiot_env_test:
            return "-test";
        case e_aiot_env_demo:
            return "-demo";
        case e_aiot_env_pro:
            return "";
        default:
        return NULL;
    }
}

int load_config_from_http(sun_aiot_context* psac, const char* purl)
{
    return 0;
}

int load_config_from_file(sun_aiot_context* psac, const char* path)
{
    return 0;
}

int get_http_url_from_path(sun_aiot_context* psac, const char* format, const char* path, char* http_url, int url_size)
{
    lbinfo("get_http_url_from_path begin\n");
    if(NULL == path || NULL == http_url || url_size <= 0)
    {
        lberror("%s Invalid parameter, path:%s, http_url:%s\n", __FUNCTION__, path, http_url);
        return -1;
    }
    lbinfo("%s(psac:%p, path:%s, http_url:%s, url_size:%d), psac->atc.access_token:%p\n", __FUNCTION__, psac, path, http_url, url_size, psac->atc.access_token);
    if(NULL == format)
    {
        format = SUN_AIOT_API_DEVICE_DEFAULT_URL_FORMAT;
    }
    memset(http_url, 0, url_size);
    //lbinfo("memset(http_url, 0, url_size)\n");
    if(memcmp(path, "http", 4) == 0)
    {
        if(strlen(path) < url_size - 1)
        {
            memcpy(http_url, path, strlen(path) + 1);
            return strlen(http_url);
        }
        else
        {
            lberror("%s not enought http_url buffer size for copy, need:%d, have:%d\n", __FUNCTION__, strlen(path) + 1, url_size);
            return -1;
        }
    }
    /*else if(strlen(path) + strlen(format) + sizeof(SUN_AIOT_API_DEVICE_PARAM_FORMAT) + MAX_HOST_TAG_SIZE > url_size)
    {
        lberror("%s not enought http_url buffer size for copy, need:%d, have:%d\n", __FUNCTION__, strlen(path) + strlen(SUN_AIOT_API_URL_FORMAT) + MAX_HOST_TAG_SIZE, url_size);
    }*/
    sprintf(http_url, format, get_host_tag_from(psac->env_type), path);
    lbinfo("sprintf(http_url:%s, format:%s, get_host_tag_from(psac->env_type):%s, path:%s)\n", http_url, format, get_host_tag_from(psac->env_type), path);
    if(psac->atc.access_token)
    {
        unsigned long timestamp = lbget_sys_time();
        sprintf(http_url + strlen(http_url), SUN_AIOT_API_DEVICE_PARAM_FORMAT, psac->atc.access_token, timestamp, psac->plang);
        lbinfo("sprintf(http_url + strlen(http_url):%s, SUN_AIOT_API_DEVICE_PARAM_FORMAT:%s, psac->atc.access_token:%s, timestamp:%lu, psac->plang:%s)\n", http_url + strlen(http_url), SUN_AIOT_API_DEVICE_PARAM_FORMAT, psac->atc.access_token, timestamp, psac->plang);
    }
    /*if(psac->atc.access_token)
    {
        sprintf(http_url, format, get_host_tag_from(psac->env_type), path, psac->atc.access_token, lbget_sys_time());
        lbinfo("after get_http_url_from_path, psac->atc.access_token:%s, http_url:%s\n", psac->atc.access_token, http_url);
    }
    else
    {
        sprintf(http_url, format, get_host_tag_from(psac->env_type), path);
        lbinfo("after get_http_url_from_path, psac->atc.access_token:%p, http_url:%s\n", psac->atc.access_token, http_url);
    }*/
    lbinfo("%s end, http_url:%s\n", __FUNCTION__, http_url);
    //lbinfo("sprintf(http_url:%s, SUN_AIOT_API_URL_FORMAT:%s, get_host_tag_from(env_type:%d):%s, path:%s)\n", http_url, SUN_AIOT_API_URL_FORMAT, env_type, get_host_tag_from(env_type), path);
    return strlen(http_url);
}

http_response_context* lbhttp_response_context_open(char* resp, int resp_len)
{
    http_response_context* phrc = NULL;
    cJSON* ptmp = NULL;
    if(NULL == resp || resp_len <= 0)
    {
        lberror("[%s] Invalid parameter, resp:%p, resp_len:%d\n", __FUNCTION__, resp, resp_len);
        return NULL;
    }

    //lbinfo("%s(resp:%s, resp_len:%d)\n", __FUNCTION__, resp, resp_len);
    do
    {
        phrc = (http_response_context*)lbmalloc(sizeof(http_response_context));
        memset(phrc, 0, sizeof(http_response_context));
        lbstrcp(phrc->response, resp);
        phrc->json_root = cJSON_Parse(resp);
        lbmem_add_ref(phrc->json_root, sizeof(cJSON));
        //lbinfo("phrc->json_root:%p = cJSON_Parse(resp:%s)\n", phrc->json_root, resp);
        if(NULL == phrc->json_root)
        {
            lberror("phrc->json_root:%p = cJSON_Parse(resp:%s), parser http response failed\n", phrc->json_root, resp);
            break;
        }

        // get http statcCode
        ptmp = cJSON_GetObjectItem(phrc->json_root, "stateCode");
        if(NULL == ptmp)
        {
            lberror("ptmp:%p = cJSON_GetObjectItem(phrc->json_root:%p, stateCode) failed\n", ptmp, phrc->json_root);
            break;
        }
        phrc->stateCode = ptmp->valueint;

        // get http state Message
        ptmp = cJSON_GetObjectItem(phrc->json_root, "stateMsg");
        if(ptmp)
        {
            lbstrcp(phrc->stateMsg, ptmp->valuestring);
        }

        // get http data object
        phrc->json_data = cJSON_GetObjectItem(phrc->json_root, "data");

        // get http obj object
        phrc->json_obj = cJSON_GetObjectItem(phrc->json_root, "object");
        return phrc;
    } while (0);

    lbhttp_response_context_close(&phrc);
    return phrc;
}

void lbhttp_response_context_close(http_response_context** pphrc)
{
    if(pphrc && *pphrc)
    {
        http_response_context* phrc = *pphrc;
        //lbinfo("%s(pphrc:%p)\n", __FUNCTION__, pphrc);
        cJSON_Delete(phrc->json_root);
        lbmem_rm_ref(phrc->json_root);
        lbfree(phrc->stateMsg);
        lbfree(phrc->response);
        lbfree(phrc);
        *pphrc = NULL;
    }
}

int http_send_and_get_response(sun_aiot_context* psac, const char* format, const char* path, const char* req, int* phttp_code, char* resp, int resp_len)
{
    int url_len = 0;
    char* http_url = NULL;
    lbinfo("%s(psac:%p, format:%p, path:%s, req:%s, phttp_code:%p, resp:%p, resp_len:%d)\n", __FUNCTION__, psac, format, path, req, phttp_code, resp, resp_len);
    if(NULL == path || NULL == req)
    {
        lberror("%s Invalid parameter, path:%s, req:%s\n", __FUNCTION__, path, req);
        return -1;
    }
    lbmutex_lock(psac->pmutex);
    do{
        http_url = (char*)lbmalloc(MAX_RESPONSE_SIZE);
        memset(http_url, 0, sizeof(MAX_RESPONSE_SIZE));
        url_len = get_http_url_from_path(psac, format, path, http_url, MAX_RESPONSE_SIZE);
        lbinfo("url_len:%d = get_http_url_from_path(psac:%p, NULL, path:%s, http_url:%s, MAX_RESPONSE_SIZE)\n", url_len, psac, path, http_url);
        if(url_len <= 0)
        {
            lberror("%s failed, url_len:%d = get_http_url_from_path(psac:%p, path:%s, http_url:%p, MAX_RESPONSE_SIZE:%d) failed\n", __FUNCTION__, url_len, psac, path, http_url, MAX_RESPONSE_SIZE);
            lbfree(http_url);
            resp_len = -1;
            break;
        }
        //lbinfo("url_len:%d = get_http_url_from_path(env_type:%d, path:%s, http_url:%s, MAX_RESPONSE_SIZE)\n", url_len, psac->env_type, path, http_url);

        resp_len = lbhttp_post_request(http_url, req, phttp_code, resp, resp_len);
        lbinfo("resp_len:%d = lbhttp_post_request(http_url:%s, req:%s, phttp_code:%d, resp:%s, resp_len:%d)\n", resp_len, http_url, req, phttp_code, resp, resp_len);
    }while(0);
    //lbcheck_return(ret, "ret = lbhttp_post_request(http_url, req, phttp_code, resp:, resp_len:%d)\n", ret, http_url, req, phttp_code, resp, resp_len);
    lbfree(http_url);
    lbmutex_unlock(psac->pmutex);
    return resp_len;
}

int http_post(sun_aiot_context* psac, const char* format, const char* path, const char* req, int* pstatus_code, http_response_context** pphrc)
{
    int http_code = 0;
    char* resp = (char*)lbmalloc(MAX_RESPONSE_SIZE);
    //char resp[MAX_RESPONSE_SIZE];
    lbinfo("%s(psac:%p, path:%s, req:%p, pstatus_code:%p, pphrc:%p)\n", __FUNCTION__, psac, path, req, pstatus_code, pphrc);
    //lbinfo("%s(psac:%p, path:%p, req:%p, pstatus_code:%p, pphrc:%p)\n", __FUNCTION__, psac, path, req, pstatus_code, pphrc);
    int ret = http_send_and_get_response(psac, format, path, req, &http_code, resp, MAX_RESPONSE_SIZE);
    lbinfo("%s ret:%d = http_send_and_get_response\n", __FUNCTION__, ret);
    if(ret <= 0 || HTTP_OK != http_code)
    {
        lbinfo("before lbfree(resp");
        lbfree(resp);
        lberror("%s ret:%d = http_send_and_get_response(psac:%p, path:%s, req:%s, http_code:%d) failed\n", __FUNCTION__, ret, psac, path, req, http_code);
        return -1;
    }
    lbinfo("before lbhttp_response_context_open");
    http_response_context* phrc = lbhttp_response_context_open(resp, ret);
    if(NULL == phrc)
    {
        lberror("%s phrc:%p = lbhttp_response_context_open(resp:%s, ret:%d)\n", __FUNCTION__, phrc, resp, ret);
        lbfree(resp);
        return -1;
    }
    lbfree(resp);
    if(pstatus_code)
    {
        *pstatus_code = phrc->stateCode;
    }

    if(pphrc)
    {
        *pphrc = phrc;
    }
    else
    {
        lbhttp_response_context_close(&phrc);
    }
    
    return 0;
}

int esp_http_send_and_get_response(sun_aiot_context* psac, const char* format, const char* path, const char* req, int* phttp_code, char* resp, int size)
{
    int url_len = 0;
    char* http_url = NULL;
    int resp_len = 0;
    lbmutex_lock(psac->pmutex);
    do{
        http_url = (char*)lbmalloc(MAX_RESPONSE_SIZE);
        memset(http_url, 0, sizeof(MAX_RESPONSE_SIZE));
        url_len = get_http_url_from_path(psac, format, path, http_url, MAX_RESPONSE_SIZE);
        lbinfo("url_len:%d = get_http_url_from_path(psac:%p, NULL, path:%s, http_url:%s, MAX_RESPONSE_SIZE)\n", url_len, psac, path, http_url);
        if(url_len <= 0)
        {
            lberror("%s failed, url_len:%d = get_http_url_from_path(psac:%p, path:%s, http_url:%p, MAX_RESPONSE_SIZE:%d) failed\n", __FUNCTION__, url_len, psac, path, http_url, MAX_RESPONSE_SIZE);
            lbfree(http_url);
            resp_len = -1;
            break;
        }
        //lbinfo("url_len:%d = get_http_url_from_path(env_type:%d, path:%s, http_url:%s, MAX_RESPONSE_SIZE)\n", url_len, psac->env_type, path, http_url);

        resp_len = esp_http_client_post(http_url, HTTP_CONTENT_TYPE_JSON, req, strlen(req), phttp_code, resp, size);
        lbinfo("resp_len:%d = lbhttp_post_request(http_url:%s, req:%s, phttp_code:%d, resp:%s, resp_len:%d)\n", resp_len, http_url, req, phttp_code, resp, resp_len);
    }while(0);
    //lbcheck_return(ret, "ret = lbhttp_post_request(http_url, req, phttp_code, resp:, resp_len:%d)\n", ret, http_url, req, phttp_code, resp, resp_len);
    lbfree(http_url);
    lbmutex_unlock(psac->pmutex);

    return resp_len;
}

int esp_http_post(sun_aiot_context* psac, const char* format, const char* path, const char* req, http_response_context** pphrc)
{
    int http_code = 0;
    int resp_len = 0;
     http_response_context* phrc = NULL;
    char* resp = (char*)malloc(MAX_RESPONSE_SIZE);
    memset(resp, 0, MAX_RESPONSE_SIZE);
    resp_len = esp_http_send_and_get_response(psac, format, path, req, &http_code, resp, MAX_RESPONSE_SIZE);
    if(resp_len <= 0 || HTTP_OK != http_code)
    {
        lberror("resp_len:%d = esp_http_send_and_get_response(psac:%p, format:%p, path:%s, req:%p, &http_code:%d, resp, MAX_RESPONSE_SIZE)\n", resp_len, psac, format, path, req, http_code);
        return -1;
    }
    if(resp_len > 0 && pphrc)
    {
        phrc = *pphrc = lbhttp_response_context_open(resp, resp_len);
    }
    return phrc ? 0 : -1;
}

void* http_create_download_task(sun_aiot_context* psac, const char* path, const char* dst_path, on_download_cb download_cb, void* owner)
{
    int ret = -1;
    lbhttp_context* phc = NULL;
    char* pstr = NULL;
    char* purl = NULL;
    if(NULL == path || (NULL == dst_path && NULL == download_cb))
    {
        lberror("Invalid paramer, path:%s, dst_path:%p, download_cb:%p\n", path, dst_path, download_cb);
        return NULL;
    }

    do{
        lbinfo("%s while begin\n", __FUNCTION__);
        purl = (char*)malloc(1024);
        memset(purl, 0, 1024);
        ret = gen_http_url_from_path(psac, path, purl, 1024);
        lbcheck_break(ret, "ret:%d = gen_http_url_from_path(psac:%p, path:%p, purl, 1024)", ret, psac, path);
        phc = lbhttp_open(purl);
        lbcheck_pointer_break(phc, "phc:%p = lbhttp_open(purl:%s)\n", phc, purl);
        sprintf(path, "%s.%s", dst_path, "tmp");
        ret = lbhttp_set_response_content_save_path(phc, path, dst_path, download_cb, owner);
        lbcheck_break(ret, "ret:%d = lbhttp_set_response_content_save_path(phc:%p, dst_path:%s, download_cb:%p, owner:%p)\n", ret, phc, dst_path, download_cb, owner);
        //lbinfo("ret:%d = lbhttp_set_response_content_save_path(phc:%p, dst_path:%s, download_cb:%p, owner:%p)\n", ret, phc, dst_path, download_cb, owner);
        ret = lbhttp_set_string_opt(phc, e_aiot_http_content_form, HTTP_CONTENT_TYPE_OCTET);
        lbcheck_break(ret, "ret:%d = lbhttp_set_string_opt(phc:%p, e_aiot_http_content_form, HTTP_CONTENT_TYPE_OCTET:%s)\n", ret, phc, HTTP_CONTENT_TYPE_OCTET);
        lbfree(purl);
        lbinfo("%s success, phc:%p\n", __FUNCTION__, phc);
        return phc;
    }while(0);
    
    lbfree(purl);
    lbhttp_response_context_close(&phc);
    lbinfo("%s failed, phc:%p\n", __FUNCTION__, phc);
    return phc;
}

int http_download_start_task(void* task_id)
{
    lbhttp_context* phc = (lbhttp_context*)task_id;
    lbcheck_pointer(phc, -1, "Invalid parameter, phc:%p\n", phc);

    return lbhttp_thread_start(phc);
}

/*void* http_download_start_task(const char* purl, int* phttp_code, const char* dst_path, on_download_cb download_cb, void* owner)
{
    int ret = -1;
    lbhttp_context* phc = NULL;
    char* dir = NULL, name = NULL, suffix = NULL;
    char path[256];
    do
    {
        phc = lbhttp_open(purl);
        lbcheck_pointer_break(phc, "phc:%p = lbhttp_open(purl:%s)\n", phc, purl);
        sprintf(path, "%s.%s", dst_path, "tmp");
        ret = lbhttp_set_response_content_save_path(phc, path, dst_path, download_cb, owner);
        lbcheck_break(ret, "ret:%d = lbhttp_set_response_content_save_path(phc:%p, dst_path:%s, download_cb:%p, owner:%p)\n", ret, phc, dst_path, download_cb, owner);
        //lbinfo("ret:%d = lbhttp_set_response_content_save_path(phc:%p, dst_path:%s, download_cb:%p, owner:%p)\n", ret, phc, dst_path, download_cb, owner);
        ret = lbhttp_set_string_opt(phc, e_aiot_http_content_form, HTTP_CONTENT_TYPE_OCTET);
        lbcheck_break(ret, "ret:%d = lbhttp_set_string_opt(phc:%p, e_aiot_http_content_form, HTTP_CONTENT_TYPE_OCTET:%s)\n", ret, phc, HTTP_CONTENT_TYPE_OCTET);
        //lbinfo("ret:%d = lbhttp_set_string_opt(phc, e_aiot_http_content_form, HTTP_CONTENT_TYPE_OCTET)\n", ret);
        ret = lbhttp_thread_start(phc);
        lbcheck_break(ret, "ret:%d = lbhttp_thread_start(phc:%p)", ret, phc);
        return phc;
    }while(0);
    
    lbhttp_close(&phc);
    return phc;
}*/

int http_download_get_progress(void* task_id, e_aiot_http_msg_type* msg_type, http_download_info* phdp, int* percent)
{
    lbhttp_context* phc = (lbhttp_context*)task_id;
    lbcheck_pointer(phc, -1, "Invalid parameter, phc:%p\n", phc);

    if(msg_type)
    {
        *msg_type = phc->msg_type;
    }
    
    if(phdp)
    {
        *phdp = phc->hdp;
    }

    if(percent)
    {
        *percent = phc->lpercent;
    }

    return 0;
}

int http_download_stop_task(void* task_id)
{
    return lbhttp_thread_stop((lbhttp_context*)task_id);
}

const char* get_mqtt_host_name(enum e_aiot_env_type env_type, int* pport)
{
    switch(env_type)
    {
        case e_aiot_env_dev:
            if(pport)
            {
                *pport = 9911;
            }
            return "iot-mqtt-server-dev.sunvalleycloud.com";
        case e_aiot_env_sit:
            if(pport)
            {
                *pport = 9913;
            }
            return "iot-mqtt-server-sit.sunvalleycloud.com";
        case e_aiot_env_test:
            if(pport)
            {
                *pport = 9912;
            }
            return "iot-mqtt-server-test.sunvalleycloud.com";
        case e_aiot_env_demo:
            if(pport)
            {
                *pport = 9914;
            }
            return "iot-mqtt-server-demo.sunvalleycloud.com";
        case e_aiot_env_pro:
            if(pport)
            {
                *pport = 9915;
            }
            return "iot-mqtt-server.sunvalleycloud.com";
        default:
        return NULL;
    }
}

int fetch_string_from_json(struct cJSON* pjn, const char* pkey, char** ppval)
{
    cJSON* pjtmp = NULL;
    lbcheck_value(!pjn || !pkey || !ppval, -1, "Invalid parameter, pjn:%p, pkey:%s, ppval:%p\n", pjn, pkey, ppval);
    lbfree(*ppval);
    pjtmp = cJSON_GetObjectItem(pjn, pkey);
    lbcheck_pointer(pjtmp, -1, "fetch access_token from response failed, pjtmp:%p\n", pjtmp);
    lbstrcp(*ppval, pjtmp->valuestring);
    //lbinfo("key:%s, str_val:%s\n", pkey, pjtmp->valuestring);
    return 0;
}

int fetch_int_from_json(struct cJSON* pjn, const char* pkey, int* pnval)
{
    cJSON* pjtmp = NULL;
    lbcheck_value(!pjn || !pkey || !pnval, -1, "Invalid parameter, pjn:%p, pkey:%s, pnval:%p\n", pjn, pkey, pnval);
    pjtmp = cJSON_GetObjectItem(pjn, pkey);
    lbcheck_pointer(pjtmp, -1, "fetch access_token from response failed, pjtmp:%p\n", pjtmp);

    *pnval = pjtmp->valueint;
    //lbinfo("key:%s, val:%d\n", pkey, pjtmp->valueint);
    return 0;
}

int fetch_double_from_json(struct cJSON* pjn, const char* pkey, double* pdval)
{
    cJSON* pjtmp = NULL;
    lbcheck_value(!pjn || !pkey || !pdval, -1, "Invalid parameter, pjn:%p, pkey:%s, pdval:%p\n", pjn, pkey, pdval);
    pjtmp = cJSON_GetObjectItem(pjn, pkey);
    lbcheck_pointer(pjtmp, -1, "fetch access_token from response failed, pjtmp:%p\n", pjtmp);

    *pdval = pjtmp->valuedouble;

    return 0;
}

int gen_mqtt_host_and_port(sun_aiot_context* psac, char* host, int size, int* pport)
{
    //char tmp[1024];
    lbcheck_pointer(psac, -1, "Invalid parameter, psac:%p\n", psac);

    if(host && size > sizeof(psac->pmqtt_host_format) + 3)
    {
        sprintf(host, psac->pmqtt_host_format, get_host_tag_from(psac->env_type));
    }
    else
    {
        lberror("not enough host buffer size, need:%d, have:%d\n", sizeof(psac->pmqtt_host_format) + 3, size);
        return -1;
    }
    
    if(pport)
    {
        assert(psac->env_type > e_aiot_env_default && psac->env_type <= e_aiot_env_pro);
        *pport = psac->mqtt_port_list[psac->env_type - 1];
    }
    
    return 0;
}

int gen_http_url_from_path(sun_aiot_context* psac, const char* path, char* purl, int size)
{
    if(NULL == psac || NULL == path || purl == NULL)
    {
        lberror("Invalid parameter, psac:%p, path:%s, purl:%p\n", psac, path, purl);
        return -1;
    }

    if(0 != memcmp(path, "http", 4))
    {
        if(strlen(psac->paiot_url_format) + strlen(path) +  strlen(psac->atc.access_token) + 20 > size)
        {
            lberror("not enough buffer size, need:%d, have:%d\n", strlen(psac->paiot_url_format) + strlen(path) +  strlen(psac->atc.access_token) + 20, size);
            return -1;
        }
        sprintf(purl, psac->paiot_url_format, get_host_tag_from(psac->env_type), path, psac->atc.access_token, lbget_sys_time());
        lbinfo("sprintf(url:%s, psac->paiot_url_format:%s, get_host_tag_from(psac->env_type):%s, path:%s, psac->atc.access_token:%s, lbget_sys_time():%lu)\n", purl, psac->paiot_url_format, get_host_tag_from(psac->env_type), path, psac->atc.access_token, lbget_sys_time());
        return 0;
    }

    if(size - 1 < strlen(path))
    {
        lberror("not enough buffer size, need:%d, have:%d\n", strlen(path) + 1, size);
        return -1;
    }
    else
    {
        memcpy(purl, path, strlen(path) + 1);
        lbinfo("gen_http_url_from_path purl:%s\n", purl);
        return 0;
    }
}


void http_free_ota_response(ota_resp_info** ppori)
{
    if(ppori && *ppori)
    {
        ota_resp_info* pori = *ppori;
        lbfree(pori->pfileUrl);
        lbfree(pori->pimei);
        lbfree(pori->pmd5);
        lbfree(pori->pofferId);
        lbfree(pori->ppublishTime);
        lbfree(pori->psn);
        lbfree(pori->phints);
        lbfree(pori->pversion);
        lbfree(pori->pstartTime);
        lbfree(pori->pendTime);
        lbfree(pori->pstateMsg);
        lbfree(pori);
    }
}

ota_resp_info* http_praser_ota_response(cJSON* pjdata)
{
    int ret = 0;
    ota_resp_info* pori = NULL;
    lbinfo("%s(pjdata:%p)\n", __FUNCTION__, pjdata);
    lbcheck_pointer(pjdata, NULL, "http_praser_ota_response failed, pjdata:%p\n", pjdata);
    do
    {
        pori = (ota_resp_info*)malloc(sizeof(ota_resp_info));
        memset(pori, 0, sizeof(ota_resp_info));

        ret = fetch_int_from_json(pjdata, "minPower", &pori->nminPower);
        //lbcheck_break(ret, "fetch minPower from response failed");
        //lbinfo("ret:%d = fetch_int_from_json(pjdata, minPower, &pori->nminPower:%d)\n", ret, pori->nminPower);
        ret = fetch_int_from_json(pjdata, "fileSize", &pori->nfileSize);
        lbcheck_break(ret, "fetch fileSize from response failed");
        //lbinfo("ret:%d = fetch_int_from_json(pjdata, fileSize, &pori->nfileSize:%d)\n", ret, pori->nfileSize);
        ret = fetch_int_from_json(pjdata, "flow", &pori->nflow);
        //lbcheck_break(ret, "fetch flow from response failed");
        //lbinfo("pori->nflow:%d\n", pori->nflow);
        ret = fetch_int_from_json(pjdata, "method", &pori->nmethod);
        //lbcheck_break(ret, "fetch method from response failed");
        //lbinfo("pori->nmethod:%d\n", pori->nmethod);
        //ret = fetch_int_from_json(pjdata, "stateCode", &pori->nstateCode);
        //lbcheck_break(ret, "fetch stateCode from response failed");
        //lbinfo("pori->nstateCode:%d\n", pori->nstateCode);
        ret = fetch_string_from_json(pjdata, "fileUrl", &pori->pfileUrl);
        lbcheck_break(ret, "fetch fileUrl from response failed");
        //lbinfo("pori->pfileUrl:%s\n", pori->pfileUrl);
        //ret = fetch_string_from_json(pjdata, "imei", &pori->pimei);
        //lbcheck_break(ret, "fetch imei from response failed");

        //lbinfo("pori->pimei:%s\n", pori->pimei);
        ret = fetch_string_from_json(pjdata, "md5", &pori->pmd5);
        lbcheck_break(ret, "fetch md5 from response failed");
        //lbinfo("pori->pmd5:%s\n", pori->pmd5);
        ret = fetch_string_from_json(pjdata, "offerId", &pori->pofferId);
        //lbcheck_break(ret, "fetch offerId from response failed");
        //lbinfo("pori->pofferId:%s\n", pori->pofferId);
        ret = fetch_string_from_json(pjdata, "publishTime", &pori->ppublishTime);
        //lbcheck_break(ret, "fetch publishTime from response failed");
        //lbinfo("pori->ppublishTime:%s\n", pori->ppublishTime);
        ret = fetch_string_from_json(pjdata, "sn", &pori->psn);
        lbcheck_break(ret, "fetch sn from response failed");
        //lbinfo("pori->psn:%s\n", pori->psn);

        ret = fetch_string_from_json(pjdata, "hints", &pori->phints);
        //lbcheck_break(ret, "fetch hints from response failed");
        //lbinfo("pori->phints:%s\n", pori->phints);
        ret = fetch_string_from_json(pjdata, "version", &pori->pversion);
        lbcheck_break(ret, "fetch pversion from response failed");
        //lbinfo("pori->pversion:%s\n", pori->pversion);
        //ret = fetch_string_from_json(pjdata, "startTime", &pori->pstartTime);
        //lbcheck_break(ret, "fetch startTime from response failed");
        //lbinfo("pori->pstartTime:%s\n", pori->pstartTime);
        //ret = fetch_string_from_json(pjdata, "endTime", &pori->pendTime);
        //lbcheck_break(ret, "fetch endTime from response failed");
        //lbinfo("pori->pendTime:%s\n", pori->pendTime);
        //ret = fetch_string_from_json(pjdata, "stateMsg", &pori->pstateMsg);
        //lbcheck_break(ret, "fetch stateMsg from response failed");
        //lbinfo("pori->pstateMsg:%s\n", pori->pstateMsg);
        return pori;
    } while (0);

    http_free_ota_response(&pori);
    return pori;
}

ota_resp_info* http_post_ota_request(sun_aiot_context* psac, aiot_ota_info* pota_info)
{
    int ret = -1;
    cJSON* pjr = NULL;
    char buf[20];
    char* pstr = NULL;
    int http_code = 0;
    ota_resp_info* pri = NULL;
    http_response_context* phrc = NULL;
    if(NULL == psac || NULL == pota_info)
    {
        lberror("Invalid parameter, psac:%p, pota_info:%p\n", psac, pota_info);
        return -1;
    }

    do{
        pjr = cJSON_CreateObject();
        lbmem_add_ref(pjr, sizeof(cJSON));
        lbcheck_pointer(pjr, -1, "pjr:%p = cJSON_CreateObject()", pjr);

        sprintf(buf, "%d", pota_info->state_of_charge);
        cJSON_AddStringToObject(pjr, "power", buf);
        //sprintf(buf, "%d", pota_info->region);
        //cJSON_AddStringToObject(pjr, "region", buf);
        cJSON_AddStringToObject(pjr, "sn", psac->device_sn);
        cJSON_AddStringToObject(pjr, "version", pota_info->version);
        pstr = cJSON_PrintUnformatted(pjr);
        ret = http_post(psac, NULL, SUN_AIOT_OTA_PATH, pstr, &http_code, &phrc);
        lbcheck_break(ret, "ret:%d = http_post(psac:%p, SUN_AIOT_OTA_PATH:%s, pstr:%s, &http_code:%d, &phrc:%p)", ret, psac, SUN_AIOT_OTA_PATH, pstr, http_code, phrc);
        lbinfo("ret:%d = http_post(psac:%p, SUN_AIOT_OTA_PATH:%s, pstr:%s, &http_code:%d, &phrc:%p)", ret, psac, SUN_AIOT_OTA_PATH, pstr, http_code, phrc);
        //lbinfo("resp:%s\n", phrc->response);
        if(HTTP_OK != http_code || NULL == phrc->json_data)
        {
            lberror("http_post failed, http_code:%d, json_data:%p\n", http_code, phrc->json_data);
            return NULL;
        }
        pri = http_praser_ota_response(phrc->json_data);
        lbcheck_pointer_break(pri, "pri:%p = http_praser_ota_response(phrc->json_data:%p) failed", pri, phrc->json_data);
        lbinfo("%s success, purl:%s\n", __FUNCTION__, pri->pfileUrl);
        //return pri;
    }while(0);
    
    lbfree(pstr);
    lbhttp_response_context_close(&phrc);
    return pri;
}