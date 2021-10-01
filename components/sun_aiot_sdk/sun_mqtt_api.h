/****************************************************************************************************************
 * filename     sun_aiot_mqtt_api.h
 * describe     Sunvalley aiot platform mqtt sdk api definition
 * author       Created by dawson.wu on 2021/01/28
 * Copyright    ©2007 - 2029 Sunvally.com.cn All Rights Reserved.
 ***************************************************************************************************************/
#pragma once
#include "mqtt_client.h"
#include "sun_aiot_api.h"

/*#ifndef _SUN_AIOT_HTTP_API_H_
typedef enum{
    // MQTT服务质量，最多发送一次
    e_mqtt_at_most_once      = 0,
    // MQTT服务质量，至少一次
    e_mqtt_at_least_once     = 1,
    // MQTT服务质量，仅仅一次
    e_mqtt_exactly_once    = 2
} e_aiot_mqtt_qos;

typedef enum
{
	e_mqtt_msg_type_unknown 			= -1,		// 未知消息类型
	e_mqtt_msg_type_get					= 0,		// 获取消息
	e_mqtt_msg_type_control				= 1,		// 控制指令消息
	e_mqtt_msg_type_delete				= 2,		// 删除消息
	e_mqtt_msg_type_update				= 3,		// 控制指令响应消息
	e_mqtt_msg_type_reply				= 4,		// 应答消息
	e_mqtt_msg_type_event_report		= 5,		// 事件上报消息
	e_mqtt_msg_type_property_report		= 6			// 设备属性上报消息
} e_aiot_mqtt_msg_type;*/
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
//typedef void (*on_mqtt_subscribe_cb)(void* handle, void * userdata, const char* ptopic, void* payload, int payload_len, e_aiot_mqtt_qos qos);
//#endif

//#endif
/*
 * @descripe：初始化mqtt实例句柄
 *
 * @param[in] pdevice_sn: 由泽宝AIOT平台生产的设备序列号（SN）
 * @param[in] pproduct_key: 泽宝AIOT平台生成的产品key
 * @param[in] pclient_secret: 泽宝AIOT平台生成的客户端密钥，如果是一机一密认证方式，则是设备密钥；如果是一型一密，则是产品密钥
 * @param[in] pcrt_path：https证书路径
 * 
 * @return 成功返回mqtt实例句柄，失败返回NULL
 */
void* sun_mqtt_init(const char* pdevice_sn, const char* pproduct_key, const char* pclient_secret, const char* pcrt_path);

/*
 * @descripe：注册mqtt事件回调函数
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] mqtt_event_cb: mqtt事件回调函数
 * @param[in] powner: mqtt回掉函数所有者指针
 * 
 * @return 成功返回0，否则为失败
 */
int sun_mqtt_register_event_handle(void* handle, mqtt_event_callback_t mqtt_event_cb, void* powner);

/*
 * @descripe: 与mqtt服务器建立连接
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] pmqtt_host: mqtt服务域名
 * @param[in] mqtt_port: mqtt服务端口
 * @param[in] keep_live_time: mqtt连接心跳时间间隔（5-60）（单位：秒），默认为30S
 * 
 * @return 成功返回0，否则为失败
 */
int sun_mqtt_connect(void* handle, const char* pmqtt_host, int mqtt_port, int keep_live_time);

/*
 * @descripe: 向指定订阅主题发送一条消息报文到MQTT服务器
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] ptopic: 接收此消息的订阅主题
 * @param[in] payload: 完整消息内容（json格式）
 * @param[in] payload_len: 消息内容长度（bytes）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0，否则为失败
 */
int sun_mqtt_pub(void* handle, const char* ptopic, const char* payload, int payload_len, e_aiot_mqtt_qos qos);

/*
 * @descripe: 快速发送一条消息报文到MQTT服务器
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] msg_type: 消息类型
 * @param[in] event: 事件名称，如果msg_type为e_mqtt_msg_type_event_report，否则为NULL
 * @param[in] command: 设备控制消息内容（"command"节点的内容，json格式）如果msg_type为e_mqtt_msg_type_control，否则为NULL
 * @param[in] data: 事件消息内容（"data"节点内容，json格式），如果msg_type为e_mqtt_msg_type_event_report，否则为NULL
 * @param[in] state: 状态/属性消息内容（"state"节点的内容，json格式）
 * @param[in] result: 设备控制消息执行结果（"result"节点内容，json格式），如果msg_type为e_mqtt_msg_type_update，否则为NULL
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_mqtt_quick_pub(void* handle, e_aiot_mqtt_msg_type msg_type, const char* event, const char* command, const char* data, const char* state, const char* result, e_aiot_mqtt_qos qos);

/*
 * @descripe: 发送一条设备控制消息报文到MQTT服务器
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] command: 控制指令消息内容（"command"节点的内容，json格式）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_mqtt_pub_ctrl(void* handle, const char* command, e_aiot_mqtt_qos qos);

/*
 * @descripe: 发送一条设备控制响应消息报文到MQTT服务器
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] state: 设备状态/属性（"state"节点的内容，json格式）
 * @param[in] result: 设备状态/属性（"state"节点的内容，json格式）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_mqtt_pub_update(void* handle, const char* state, const char* result, e_aiot_mqtt_qos qos);

/*
 * @descripe: 发送一条事件上报消息报文到MQTT服务器
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] event：事件名称
 * @param[in] data: 事件消息内容（"data"节点的内容，json格式）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_mqtt_pub_event(void* handle, const char* event, const char* data, e_aiot_mqtt_qos qos);

/*
 * @descripe: 发送一条状态/属性上报消息报文到MQTT服务器
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] state: 事件消息内容（"state"节点的内容，json格式）
 * @param[in] qos: 消息服务质量
 * 
 * @return 成功返回0，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_mqtt_pub_property(void* handle, const char* state, e_aiot_mqtt_qos qos);

/*
 * @descripe: 订阅一个主题
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] ptopic: 订阅的消息主题，默认为：/shadow/${product_key}/${device_sn}/get
 * @param[in] qos: 消息服务质量
 * @param[in] on_topic_msg: 订阅消息回调函数
 *  @param[in] userdata: 订阅消息的拥有者的指针，在回调函数中作为回调消息的拥有者传递给被调用者
 * 
 * @return 成功返回0，否则为失败
 * 
 * @remark 消息发布主题默认为：/shadow/${product_key}/${device_sn}/update
 */
int sun_mqtt_sub(void* handle, const char* ptopic, e_aiot_mqtt_qos qos, on_mqtt_subscribe_cb on_topic_msg, void* userdata);

/*
 * @descripe: 取消订阅一个主题
 *
 * @param[in] handle: mqtt实例句柄
 * @param[in] ptopic: 订阅的消息主题，默认为：/shadow/${product_key}/${device_sn}/get
 * 
 * @return 成功返回0，否则为失败
 */
int sun_mqtt_unsub(void* handle, const char* ptopic);

/*
 * @descripe: 断开连接
 *
 * @param[in] handle: mqtt实例句柄
 * 
 * @return 无
 */
void sun_mqtt_disconnect(void* handle);

/*
 * @descripe: 销毁MQTT句柄
 *
 * @param[in/out] handle: mqtt实例句柄指针，销毁后指针将被置为0
 * 
 * @return 无
 */
void sun_mqtt_deinit(void** phandle);

/*
 * @descripe: 获取MQTT SDK版本号
 *
 * @param[in] pversion: 当前SDK无符号整形版本号：major<<24 | minor << 16 | macro << 8 | tiny
 * @param[in] szver_info: MQTT版本号信息接收buffer
 * @param[in] len: szver_info 缓冲长度（字节）
 * 
 * @return 返回szver_info的有效字符长度
 */
int sun_mqtt_get_version(unsigned int* pversion, char* szver_info, int len);

//int sun_http_post(const char* purl, int status_code, char* pbody, int len);