#include "esp_http.h"
#include "stdint.h"
#include "esp_http_client.h"
#include "lbmacro.h"
#define ESP_HTTP_CLIENT_TIMEOUT_MS          10000

esp_err_t esp_http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id)
    {
        case HTTP_EVENT_ON_CONNECTED:
        {
            lbinfo("on http connect!\n");
            break;
        }
        case HTTP_EVENT_HEADERS_SENT:
        {
            lbinfo("HTTP_EVENT_HEADERS_SENT!\n");
            break;
        }
        case HTTP_EVENT_ON_HEADER:
        {
            lbinfo("HTTP_EVENT_ON_HEADER!\n");
            break;
        }
        case HTTP_EVENT_ON_DATA:
        {
            lbinfo("HTTP_EVENT_ON_DATA!\n");
            break;
        }
        case HTTP_EVENT_ON_FINISH:
        {
            lbinfo("HTTP_EVENT_ON_FINISH!\n");
            break;
        }
        case HTTP_EVENT_DISCONNECTED:
        {
            char resp[1024];
            int resp_len = 1024;
            memset(resp, 0, 1024);
            lbinfo("HTTP_EVENT_DISCONNECTED!\n");
            resp_len = esp_http_client_read_response(evt->client, evt->user_data, 1024);
            lbinfo("resp_len:%s = esp_http_client_read_response(evt->client, resp:%s, 1024)\n", resp_len, evt->user_data);
            break;
        }
        default:
        {
            lberror("Invalid event id:%d\n");
            break;
        }
    }
    return 0;
}
int esp_http_client_post(const char* purl, const char* content_type, const char* content, int content_len, int* phttp_code, char* resp, int size)
{
    esp_err_t err = ESP_OK;
    int resp_len = 0;
    int http_code = 0;
    int recv_len = 0;
    esp_http_client_handle_t client = NULL;
    esp_http_client_config_t config = {
        .url = purl,
        .cert_pem = NULL, //(char *)server_cert_pem_start,
        .timeout_ms = ESP_HTTP_CLIENT_TIMEOUT_MS,
        .keep_alive_enable = true,
        .buffer_size = 1024,
        .event_handler = esp_http_event_handle,
        .user_data = resp,
    };

    if(resp && resp_len > 0)
    {
        memset(resp, 0, resp_len);
    }
    do
    {
        client = esp_http_client_init(&config);
        lbcheck_pointer_break(client, "client:%p = esp_http_client_init(&config) failed, purl:%s\n", client, purl);
        lbinfo("client:%p = esp_http_client_init(&config)\n", client);
    
    /*err = esp_http_client_open(client, 0);
    lbcheck_return(err, "err:%d = esp_http_client_open(client:%p, 0)\n", err, client);
    lbinfo("err:%d = esp_http_client_open(client:%p, 0)\n", err, client);*/
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        lbinfo("after esp_http_client_set_method(client, HTTP_METHOD_POST)\n");
        if(content_type)
        {
            lbinfo("content_type:%s\n", content_type);
            esp_http_client_set_header(client, "Content-Type", content_type);
            lbinfo("esp_http_client_set_header(client:%p, Content-Type, content_type:%s)\n", client, content_type);
        }

        if(content_len > 0)
        {
            lbinfo("content_len:%d\n", content_len);
            esp_http_client_set_post_field(client, content, content_len);
        }
        lbinfo("before esp_http_client_perform\n");
        err = esp_http_client_perform(client);
        lbcheck_break(err, "err:%d = esp_http_client_perform\n", err);
        lbinfo("err:%d = esp_http_client_perform(client)\n", err);
        resp_len = esp_http_client_get_content_length(client);
        lbinfo("resp_len:%d = esp_http_client_get_content_length(client)\n, resp_len", resp_len);
        /*err = esp_http_client_open(client, content_len);
        lbcheck_break(err, "err:%d = esp_http_client_open(client:%p, content_len:%d)\n", err, client, content_len);
    
        resp_len = esp_http_client_fetch_headers(client);*/
        lbcheck_value_break(resp_len <= 0 || resp_len > size, "resp_len:%d = esp_http_client_fetch_headers(client) failed, have:%d\n", resp_len, size);
        recv_len = esp_http_client_read_response(client, resp, resp_len);
        lbinfo("recv_len:%d = esp_http_client_read_response(client:%p, resp:%s, resp_len:%d)\n", recv_len, client, resp, resp_len);
        lbcheck_value_break(recv_len < resp_len, "recv_len:%d = esp_http_client_read_response(client:%p, resp:%s, resp_len:%d) failed\n", recv_len, client, resp, resp_len);
        http_code = esp_http_client_get_status_code(client);
        if(phttp_code)
        {
            *phttp_code = http_code;
        }
    } while (0);
    
    esp_http_client_cleanup(client);
    lbinfo("%s http_code:%d, resp_len:%d, resp:%s\n", __FUNCTION__, http_code, recv_len, resp);
    return recv_len;
}