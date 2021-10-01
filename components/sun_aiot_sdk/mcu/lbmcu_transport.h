#pragma once
#include "lbmcu_cmd.h"
/*
* @descripe：事件回调函数
*
* @param[in] powner: 回调函数所有者指针
* @param[in] msg_id: 消息id
* @param[in] wparam：消息辅助说明参数
* @param[in] lparam：消息值
*
* @return 无
*/
typedef void (*event_callback)(void* powner, long msg_id, long wparam, long lparam);

void* mcu_transport_init(int uart_num);

//int mcu_transport_open(int uart_num);

int mcu_transport_set_heart_beat_interval(void* handle, int interval_ms);

int mcu_transport_set_event_notify(void* handle, event_callback event_cb, void* powner);

int mcu_transport_start(void* handle);

int mcu_transport_stop(void* handle);

void mcu_transport_deinit(void** pphandle);

int mcu_transport_write_frame(void* handle, mcu_cmd_frame_t* pmcf);

mcu_cmd_frame_t* mcu_transport_read_frame(void* handle);

int on_recv_mcu_frame(void* handle, mcu_cmd_frame_t* pmcf);