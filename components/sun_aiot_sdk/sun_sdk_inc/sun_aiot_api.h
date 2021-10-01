#ifndef _SUN_AIOT_API_H_
#define _SUN_AIOT_API_H_
#include "mqtt_client.h"
//#include "../mqtt_client.h"
//#include "sun_aiot_mqtt_api.h"
#include <stdint.h>
typedef enum e_aiot_env_type{
    e_aiot_env_unknown      = -1,
    e_aiot_env_default      = 0,
    e_aiot_env_dev          = 1,
    e_aiot_env_sit          = 2,
    e_aiot_env_test         = 3,
    e_aiot_env_demo         = 4,
    e_aiot_env_pro          = 5
} e_aiot_env_type;

// 日志等级枚举类型
typedef enum{
    e_log_level_verb        = 1,
    e_log_level_info        = 2,
    e_log_level_trace       = 3,
    e_log_level_warn        = 4,
    e_log_level_error       = 5,
    e_log_level_disable     = 6
} e_log_level_type;

typedef enum{
    // MQTT服务质量，最多发送一次
    e_aiot_mqtt_at_most_once      = 0,
    // MQTT服务质量，至少一次
    e_aiot_mqtt_at_least_once     = 1,
    // MQTT服务质量，仅仅一次
    e_aiot_mqtt_accurate_once    = 2
} e_aiot_mqtt_qos;

typedef enum
{
	e_aiot_mqtt_msg_type_unknown 			    = -1,		// 未知消息类型
	e_aiot_mqtt_msg_type_get					= 0,		// 获取消息
	e_aiot_mqtt_msg_type_control				= 1,		// 控制指令消息
	e_aiot_mqtt_msg_type_delete				    = 2,		// 删除消息
	e_aiot_mqtt_msg_type_update				    = 3,		// 控制指令响应消息
	e_aiot_mqtt_msg_type_reply				    = 4,		// 应答消息
	e_aiot_mqtt_msg_type_event_report		    = 5,		// 事件上报消息
	e_aiot_mqtt_msg_type_property_report		= 6			// 设备属性上报消息
} e_aiot_mqtt_msg_type;

typedef struct{
    long    ldownload_speed;    // in bits per second
    long    ldownload_bytes;    // current http file download bytes
    long    ltotal_bytes;       // total http file download bytes
    long    lspend_time_ms;     // http download in millisecond
    //char*   pdata;              // current download data
    //int     ndata_len;          // pdata valid datalen 
} http_download_info;

typedef struct{
    int     state_of_charge;            // 电池电量
    int     region;                     // 地区
    char    version[12];                // 版本号，六位或者八位，00.00.00 或 00.00.00.00
} aiot_ota_info;

typedef enum{
    e_aiot_ota_msg_type_error                       = -1,       // an error occure when ota, wparam:errno(), lparam:error code
    e_aiot_ota_msg_type_prepared                    = 0,        // ota prepare, wparam:0, lparam:0
    e_aiot_ota_msg_type_connected                   = 1,        // ota http reuqest connecte, wparam:0, lparam:0
    e_aiot_ota_msg_type_download_progress           = 2,        // ota package in download progress, wparam:current download speed(bps), lparam:current download percent
    e_aiot_ota_msg_type_download_complete           = 3,        // ota package download complete, wparam:0, lparam:0
    e_aiot_ota_msg_type_ota_end                     = 4,        // ota package have been download and write to flash, wparam:0, lparam:0
    e_aiot_ota_msg_type_reboot                      = 5,        // ota task end, reboot the system, wparam:0, lparam:0
} e_aiot_ota_msg_type;


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
* @descripe：OTA任务回调函数
*
* @param[in] task_id: OTA任务任务taskid
* @param[in] userdata: OTA调用者的对象指针
* @param[in] msg_id: OTA消息id
* @param[in] wparam：OTA消息辅助说明参数
* @param[in] lparam：OTA消息值
*
* @return 无
*/
typedef void (*on_ota_msg_cb)(void* powner, long msg_id, long wparam, long lparam);

//typedef void (*on_http_msg_cb)(void* task_id, void* powner, long msg_id, long wparam, long lparam);
/*
* @descripe：下载回调函数
*
* @param[in] task_id: http下载任务taskid
* @param[in] userdata: 下载调用者的对象指针
* @param[in] speed：下载速度，bits/秒
* @param[in] download_bytes: 当前已下载字节数
* @param[in] toatal_bytes: 总共下载字节数
* @param[in] spend_time_ms: 当前使用时间（单位/毫秒）
*
* @return 无
*/

//typedef void (*on_download_cb)(void* task_id, void* userdata, long speed, long download_bytes, long toatal_bytes, long spend_time_ms);

/*
* @descripe：mqtt事件回调通知函数
*
* @param[in] handle: 调用者指针
* @param[in] const char* base: 
* @param[in] int32_t event_id: 事件id
* @param[in] void *event_data: 事件内容结构体
*
* @return 无
*/
//typedef void (*on_mqtt_event_cb)(void *owner, const char* base, int32_t event_id, void *event_data);

/*
* @descripe：订阅主题回调消息函数定义
*
* @param[in] handle: mqtt实例句柄
* @param[in] userdata: mqtt订阅主题调用者的对象指针
* @param[in] ptopic: 订阅消息主题
* @param[in] payload: 订阅消息负载内容
* @param[in] payload_len: 订阅消息负载内容的长度(bytes)
* @param[in] 消息服务质量
*
* @return 无
*/

typedef void (*on_mqtt_subscribe_cb)(void* handle, void * userdata, const char* ptopic, void* payload, int payload_len, e_aiot_mqtt_qos qos);

/*
 * @descripe: aiot实例句柄创建接口 
 * @param[in] client_id: 租户id，对应product key/产品key
 * @param[in] client_secret: 租户密钥，对应device_secret，设备密钥
 * @param[in] device_sn: 设备SN
 * @param[in] pcrt_path：证书路径
 * @param[in] env_type：aiot环境，详见e_aiot_env_type，默认为e_aiot_env_default（0），由sdk根据sn特性判断该设备SN所在的环境
 * @return 成功返回aiot实例句柄，失败返回NULL
 */
void* sun_aiot_api_init(const char* client_id, const char* client_secret, const char* device_sn, const char* pcrt_path, e_aiot_env_type env_type);

/*
 * @descripe: aiot实例句柄销毁接口 
 *
 * @param[in/out] handle: aiot实例句柄
 * 
 * @return 无
 */
void sun_aiot_api_deinit(void** handle);


/*
 * @descripe: aiot sdk加载配置文件
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] purl: 配置文件路径，可为NULL， 如果为NULL，则使用默认配置
 * 
 * @return 成功返回0，失败返回非零
 * @remark 此接口为预留接口，sdk通过此接口可以从AIOT平台加载固件配置信息
 */
int sun_aiot_api_load_config(void* handle, const char* purl);

/*
 * @descripe: aiot sdk日志输出设置接口 
 *
 * @param[in] level: 日志等级 1：verbose（非常详细的日志）；2：info（详细日志）；3：trace（简要日志）；4：warn（警告日志）5：error（错误日志）；6：disable（禁用所有日志）
 * @param[in] mode: 日志输出模式 1:控制台输出； 2：文件输出； 3：控制台与文件同时输出
 * @param[in] path: 文件日志输出路径，仅当mode&2为真的情况下路径设置才会生效，如果路径设置为NULL，就算mode&2为真也不会输出日志
 * 
 * @return 成功返回0，失败返回非零
 */
int sun_aiot_api_init_log(int level, int mode, const char* path);

/*
 * @descripe: 登陆aiot平台
 *
 * @param[in] handle: aiot实例句柄
 * @return 成功返回0，否则为失败
 * @reference http://cs-showdoc.sunvalley.com.cn/web/#/13?page_id=1153
 */
int sun_aiot_api_connect(void* handle);

/*
 * @descripe: 登出aiot平台
 *
 * @param[in] handle: aiot实例句柄
 * 
 * @return 无
 * @reference http://cs-showdoc.sunvalley.com.cn/web/#/13?page_id=1153
 */
void sun_aiot_api_disconnect(void* handle);

/*
 * @descripe: aiot 平台http接口post方式访问
 *sun_aiot_api_mqtt_pub_event
 * @param[in] handle: aiot实例句柄
 * @param[in] path: http方法访问路径
 * @param[in] req, http post request body
 * @param[in] phttp_code: http post error code
 * @param[in] resp: http post response body
 * @param[in] len: http post response body length
 * 
 * @return 成功返回0，否则为失败
 */
int sun_aiot_api_http_post(void* handle, const char* path, const char* req, int* phttp_code, char* resp, int len);


/*
 * @descripe: 创建ota升级包下载任务
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] path: http方法访问路径
 * @param[in] dst_path: http 文件下载路径
 * @param[in] download_cb: http 下载进度回调通知函数
 * @param[in] owner, http download cb owner
 * @param[in] pota_info, ota info struct ptr
 * @return 成功返回非零的taskid，失败返回NULL
 */
void* sun_aiot_api_create_oat_task(void* handle, aiot_ota_info* pota_info);

/*
 * @descripe: 设置下载任务回调通知函数
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] download_cb: http 下载进度回调通知函数
 * @param[in] owner, http download cb owner
 * @return 成功返回0，否则为失败
 */
int sun_aiot_api_set_ota_callback(void* task_id, on_ota_msg_cb ota_cb, void* owner);

/*
 * @descripe: 创建http下载路径
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] path: http方法访问路径
 * @param[in] dst_path: http 文件下载路径
 * @param[in] download_cb: http 下载进度回调通知函数
 * @param[in] owner, http download cb owner
 * @return 成功返回非零的taskid，失败返回NULL
 */
//void* sun_aiot_api_create_http_download_task(void* handle, const char* path, const char* dst_path, on_http_msg_cb download_cb, void* owner);


/*
 * @descripe: aiot 平台http下载接口
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] path: http方法访问路径
 * @param[in] dst_path: http 文件下载路径
 * @param[in] download_cb: http 下载进度回调通知函数
 * 
 * @return 成功返回非零的taskid，失败返回NULL
 */
int sun_aiot_api_ota_start(void* task_id);
//void* sun_aiot_api_http_download_start(void* handle, const char* path, int* phttp_code, const char* dst_path, on_http_msg_cb download_cb, void* owner);

/*
 * @descripe: http下载任务获取当前下载进度
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] task_id: http下载任务id
 * @param[in] speed：http下载速度，bits/S
 * @param[out] download_bytes：当前下载字节数，可以为NULL
 * @param[out] total_bytes：总下载字节数，可以为NULL
 * @param[out] spend_time_ms:http下载消耗时间，单位为毫秒, 可为NULL，可以调用者根据此时间预估下载剩余时间
 * 
 * @return 返回当前下载进度百分比
 */

/*
 * @descripe: http下载任务获取当前下载进度
 *
 * @param[in] task_id: http下载任务id
 * @param[out] msg_type：http下载状态消息类型
 * @param[out] phdp：http下载辅助说明信息结构体，详见http_download_param定义
 * @param[out] percent：当前下载任务进度（百分比）
 * 
 * @return 返回当前下载进度百分比
 * remark：本接口与http回调接口on_http_msg_cb的回调参数对应，调用者可以二选一使用
 */
int sun_aiot_api_ota_get_progress(void* task_id, e_aiot_ota_msg_type* msg_type, http_download_info* phdp, int* percent);
//int sun_aiot_api_http_download_get_progress(void* task_id, long* speed, long* download_bytes, long* total_bytes, long* spend_time_ms);
/*
 * @descripe: 停止并销毁http下载任务
 *
 * @param[in] task_id: http下载任务id
 * 
 * @return 成功返回0，否则为失败
 */
int sun_aiot_api_ota_stop(void* task_id);

/*
 * @descripe: 获取aiot sdk版本信息
 *
 * @param[out] char* szver: 接收字符串版本信息的buffer长度
 * @param[in] int len: http方法访问路径
 * 
 * @return 成功返回0，否则为失败
 */
int sun_aiot_api_version(char* szver, int len);

/*************************************************************** mqtt api *******************************************************************/

int sun_aiot_api_mqtt_register_event_handle(void* handle, esp_event_handler_t mqtt_event_cb, void* powner);
/*
 * @descripe: 向指定订阅主题发送一条消息报文到MQTT服务器
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] ptopic: 接收此消息的订阅主题
 * @param[in] payload: 完整消息内容（json格式）
 * @param[in] payload_len: 消息内容长度（bytes）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0值，否则为失败
 */
int sun_aiot_api_mqtt_pub(void* handle, const char* ptopic, const char* payload, int payload_len, e_aiot_mqtt_qos qos);

/*
 * @descripe: 发送一条设备控制消息报文到MQTT服务器
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] command: 控制指令消息内容（"command"节点的内容，json格式）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0值，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_aiot_api_mqtt_pub_ctrl(void* handle, const char* command, e_aiot_mqtt_qos qos);

/*
 * @descripe: 发送一条设备控制响应消息报文到MQTT服务器
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] state: 设备状态/属性（"state"节点的内容，json格式）
 * @param[in] result: 设备状态/属性（"state"节点的内容，json格式）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0值，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_aiot_api_mqtt_pub_update(void* handle, const char* state, const char* result, e_aiot_mqtt_qos qos);

/*
 * @descripe: 发送一条事件上报消息报文到MQTT服务器
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] event：事件名称
 * @param[in] data: 事件消息内容（"data"节点的内容，json格式）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0值，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_aiot_api_mqtt_pub_event(void* handle, const char* event, const char* data, e_aiot_mqtt_qos qos);

/*
 * @descripe: 发送一条状态/属性上报消息报文到MQTT服务器
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] state: 事件消息内容（"state"节点的内容，json格式）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0值，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_aiot_api_mqtt_pub_property(void* handle, const char* state, e_aiot_mqtt_qos qos);

/*
 * @descripe: 订阅一个主题
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] ptopic: 订阅的消息主题，默认为：/shadow/${product_key}/${device_sn}/get
 * @param[in] qos: 消息服务质量
 * @param[in] on_topic_msg: 订阅消息回调函数
 *  @param[in] userdata: 订阅消息的拥有者的指针，在回调函数中作为回调消息的拥有者传递给被调用者
 * 
 * @return 成功返回0值，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_aiot_api_mqtt_sub(void* handle, const char* ptopic, e_aiot_mqtt_qos qos, on_mqtt_subscribe_cb on_topic_msg, void* userdata);

/*
 * @descripe: 取消订阅一个主题
 *
 * @param[in] handle: aiot实例句柄
 * @param[in] ptopic: 订阅的消息主题，默认为：/shadow/${product_key}/${device_sn}/get
 * 
 * @return 成功返回0值，否则为失败
 */
int sun_aiot_api_mqtt_unsub(void* handle, const char* ptopic);

#endif