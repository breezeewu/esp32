#ifndef _LAZY_BEAR_HTTP_H_
#define _LAZY_BEAR_HTTP_H_
#include <stdio.h>
#include "lburl.h"
#include "sun_aiot_api.h"
//#include "lbtcp.h"
#define HTTP_OK         200
#define HTTP_VERSION                            "HTTP/1.1"
#define LBSP_HTTP_CRLF                          "\r\n"
#define HTTP_ATTRIBUTE_HOST                     "Host"
#define HTTP_ATTRIBUTE_CONNECTION               "Connection"
#define HTTP_ATTRIBUTE_CONTENT_LENGTH           "Content-Length"
#define HTTP_ATTRIBUTE_USER_AGENT               "User-Agent"
#define HTTP_ATTRIBUTE_SRS_SERVER_NAME          "srs-server-name"
#define HTTP_ATTRIBUTE_CONTENT_TYPE             "Content-Type"
#define HTTP_ATTRIBUTE_DATE                     "Date"
#define HTTP_ATTRIBUTE_TRANSFER_ENCODING        "Transfer-Encoding"
#define HTTP_SUNVALLEY "sunvalley librtmp " << "V2020.06.09"

// content-type
// json string body
#define HTTP_CONTENT_TYPE_JSON                          "application/json;charset=UTF-8"

// octet file body
#define HTTP_CONTENT_TYPE_OCTET                         "application/octet-stream"
//typedef void (*on_download_callback)(void* task_id, void* userdata, long speed, long download_bytes, long toatal_bytes, long spend_time_ms);

#ifndef _SUN_AIOT_API_H_
typedef struct{
    long    ldownload_speed;    // in bits per second
    long    ldownload_bytes;    // current http file download bytes
    long    ltotal_bytes;       // total http file download bytes
    long    lspend_time_ms;     // http download in millisecond
} http_download_info;

typedef enum{
    e_aiot_http_msg_on_connect_complete             = 0,    // http connect complete, wparam:0, lparam:0
    e_aiot_http_msg_on_request_header_complete      = 1,    // request send complete, wparam:0, lparam:0
    e_aiot_http_msg_on_request_complete             = 2,    // request send complete, wparam:0, lparam:0
    e_aiot_http_msg_on_response_header_complete     = 3,    // response send complete, wparam:0, lparam:0
    e_aiot_http_msg_on_response_body_progress       = 4,    // http resonse body receive progress, wparam:http_download_info ptr, lparam:http download percent
    e_aiot_http_msg_on_response_complete            = 5,    // http download complete, wparam:http_download_info ptr, lparam:http download percent
    e_aiot_http_msg_on_connect_error                = 20,   // http download error, wparam: error message, can be null, lparam: error code
    e_aiot_http_msg_on_request_error                = 21,   // http download error, wparam: error message, can be null, lparam: error code
    e_aiot_http_msg_on_response_error               = 22    // http download error, wparam: error message, can be null, lparam: error code
} e_aiot_http_msg_type;

/*
* @descripe：下载回调函数
*
* @param[in] task_id: http下载任务taskid
* @param[in] userdata: 下载调用者的对象指针
* @param[in] msg_id:http消息id
* @param[in] wparam：http消息辅助说明参数
* @param[in] lparam：http消息值
*
* @return 无
*/
typedef void (*on_http_msg_cb)(void* task_id, void* userdata, long msg_id, long wparam, long lparam);
#endif
/*typedef enum{
    e_aiot_http_msg_on_connect_complete             = 0,    // http connect complete, wparam:0, lparam:0
    e_aiot_http_msg_on_request_header_complete      = 1,    // request send complete, wparam:0, lparam:0
    e_aiot_http_msg_on_request_complete             = 2,    // request send complete, wparam:0, lparam:0
    e_aiot_http_msg_on_response_header_complete     = 3,    // response send complete, wparam:0, lparam:0
    e_aiot_http_msg_on_response_body_progress       = 4,    // http resonse body receive progress, wparam:http_download_info ptr, lparam:http download percent
    e_aiot_http_msg_on_response_complete            = 5,    // http download complete, wparam:http_download_info ptr, lparam:http download percent
    e_aiot_http_msg_on_connect_error                = 20,   // http download error, wparam: error message, can be null, lparam: error code
    e_aiot_http_msg_on_request_error                = 21,   // http download error, wparam: error message, can be null, lparam: error code
    e_aiot_http_msg_on_response_error               = 22    // http download error, wparam: error message, can be null, lparam: error code
} e_aiot_http_msg_type;

#define HTTP_PROGRESS_CONNECT_COMPLETE          0
#define HTTP_PROGRESS_SEND_REQUEST_HEADER       1
#define HTTP_PROGRESS_SEND_REQUEST_CONTENT      2
#define HTTP_PROGRESS_RECV_RESPONSE_HEADER      3
#define HTTP_PROGRESS_RECV_RESPONSE_BODY        4
#define HTTP_PROGRESS_RECV_RESPONSE_COMPLETE    5
#define HTTP_PROGRESS_CONNECT_ERROR             20
#define HTTP_PROGRESS_SEND_REQUEST_ERROR        21
#define HTTP_PROGRESS_RECV_RESPONSE_ERROR       22*/
typedef enum{
    e_aiot_http_unknown             = -1,
    e_aiot_http_method              = 0,
    e_aiot_http_host                = 1,
    e_aiot_http_content_form        = 2,
    e_aiot_http_content_length      = 3,
    e_aiot_http_content_body        = 4
} e_aiot_http_option;

typedef enum{
    // no content
    e_aiot_http_conent_none         = 0,

    // string/json content
    e_aiot_http_content_string      = 1,

    // file content
    e_aiot_http_content_file        = 2,

    // octet content
    e_aiot_http_content_octet       = 3
} e_aiot_http_content_type;

typedef struct{
    lburl_context*  puc;

    // http header
    char*           pmethod;
    char*           phost;
    char*           pconnection;
    char*           pcontent_type;
    long            lcontent_length;
    long            lsend_bytes;

    // http content
    char*           pcontent_body;
    FILE*           pfile;
    e_aiot_http_content_type    econtent_type;

    /*// http head field
    char*           phost;
    char*           pconnection;
    char*           pcontent_type;
    int             ncontent_length;
    char*           pcontent_body;

    
    long            lspeed;
    unsigned long   ustart_time;
    unsigned long   uend_time;
    long            ldownload_bytes;
    long            ltotal_bytes;

    char*           pbody;
    int             body_len;
    int             complete;*/
} lbhttp_request_context;

typedef void (*on_http_msg_cb)(void* task_id, void* powner, long msg_id, long wparam, long lparam);

typedef struct{
    int         http_progress_status;
    long        lpercent;
    long        ltotal_bytes;
    long        lrecv_bytes;
    long        lspeed_bits_per_sec;
    unsigned long lspend_time_ms;
    
} lbhttp_progress_info;

typedef struct{
    int        nhttpCode;
    int        nstateCode;
    char*      pstateMsg;
    char*      presponse;
    char*      ptmp_path;
    char*      pdst_path;
    long       lresp_len;
    long       lcontent_length;
    /*long       lrecv_bytes;
    long       lspeed_bits_per_sec;
    unsigned long lspend_time_ms;*/
    
    FILE*      pfile;
    struct lbkv_list*  pkv_list;
    e_aiot_http_content_type    con_type;
    

} lbhttp_response_context;

typedef struct{
    //void*                   powner;
    struct lbnet_context*       pnc;
    struct lbthread_context*    phttp_thread;
    on_http_msg_cb              download_cb;
    void*                       powner;
    long                        lpercent;
    e_aiot_http_msg_type        msg_type;          
    http_download_info         hdp;
    lbhttp_request_context      request;
    lbhttp_response_context     response;
} lbhttp_context;



//int lbhttp_send_request_and_get_response(const char* purl, const char* preq, int* pcode, char* resp, int resp_len);

lbhttp_context* lbhttp_open(const char* purl);

int lbhttp_set_string_opt(lbhttp_context* phc, e_aiot_http_option opt, const char* pval);

int lbhttp_set_long_opt(lbhttp_context* phc, e_aiot_http_option opt, long lval);

int lbhttp_set_request_content(lbhttp_context* phc, e_aiot_http_content_type con_type, const char* pval, int content_len); // content_len为0，则需要自己计算content length

int lbhttp_set_response_content_save_path(lbhttp_context* phc, const char* tmp_path, const char* dst_path, on_http_msg_cb download_cb, void* powner);
//int lbhttp_send_header(lbhttp_context* phc);

int lbhttp_send_request(lbhttp_context* phc);

//nt lbhttp_send_buffer(lbhttp_context* phc, const char* pbuffer, int len);

int http_recv_response(lbhttp_context* phc);

//int lbhttp_recv_buffer(lbhttp_context* phc, char* pbuffer, int len);

void lbhttp_close(lbhttp_context** pphc);

int lbhttp_thread_start(lbhttp_context* phc);

int lbhttp_thread_stop(lbhttp_context* phc);

int lbhttp_post_request(const char* purl, const char* preq, int* phttp_code, char* resp, int resp_len);

int http_msg_callback(lbhttp_context* phc, int status, long wparam, long lparam);
#endif