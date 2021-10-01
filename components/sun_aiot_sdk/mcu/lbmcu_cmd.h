#pragma once
#include <stdint.h>
#define mcu_dp_type_raw        0
#define mcu_dp_type_bool        0
#define MCU_CMD_FRAME_HEADER_SIZE       7

//=============================================================================
//Byte order of the frame
//=============================================================================
#define         HEAD_FIRST                      0
#define         HEAD_SECOND                     1        
#define         PROTOCOL_VERSION                2
#define         FRAME_TYPE                      3
#define         LENGTH_HIGH                     4
#define         LENGTH_LOW                      5
#define         DATA_START                      6

//=============================================================================
//Data frame type
//=============================================================================
#define         HEAT_BEAT_CMD                   0                               //心跳包
#define         PRODUCT_INFO_CMD                1                               //产品信息
#define         WORK_MODE_CMD                   2                               //查询MCU 设定的模块工作模式	
#define         WIFI_STATE_CMD                  3                               //wifi工作状态	
#define         WIFI_RESET_CMD                  4                               //重置wifi
#define         WIFI_MODE_CMD                   5                               //选择smartconfig/AP模式	
#define         DATA_QUERT_CMD                  6                               //命令下发
#define         STATE_UPLOAD_CMD                7                               //状态上报	 
#define         STATE_QUERY_CMD                 8                               //状态查询   
#define         UPDATE_START_CMD                0x0a                            //升级开始
#define         UPDATE_TRANS_CMD                0x0b                            //升级传输
#define         GET_ONLINE_TIME_CMD             0x0c                            //获取系统时间(格林威治时间)
#define         FACTORY_MODE_CMD                0x0d                            //进入产测模式    
#define         WIFI_TEST_CMD                   0x0e                            //wifi功能测试
#define         GET_LOCAL_TIME_CMD              0x1c                            //获取本地时间
#define         WEATHER_OPEN_CMD                0x20                            //打开天气          
#define         WEATHER_DATA_CMD                0x21                            //天气数据
#define         STATE_UPLOAD_SYN_CMD            0x22                            //状态上报（同步）
#define         STATE_UPLOAD_SYN_RECV_CMD       0x23                            //状态上报结果通知（同步）
#define         HEAT_BEAT_STOP                  0x25                            //关闭WIFI模组心跳
#define         STREAM_TRANS_CMD                0x28                            //流数据传输
#define         GET_WIFI_STATUS_CMD             0x2b                            //获取当前wifi联网状态
#define         WIFI_CONNECT_TEST_CMD           0x2c                            //wifi功能测试(连接指定路由)
#define         GET_MAC_CMD                     0x2d                            //获取模块mac
#define         GET_IR_STATUS_CMD               0x2e                            //红外状态通知
#define         IR_TX_RX_TEST_CMD               0x2f                            //红外进入收发产测
#define         MAPS_STREAM_TRANS_CMD           0x30                            //流数据传输(支持多张地图)
#define         FILE_DOWNLOAD_START_CMD         0x31                            //文件下载启动
#define         FILE_DOWNLOAD_TRANS_CMD         0x32                            //文件下载数据传输
#define         MODULE_EXTEND_FUN_CMD           0x34                            //模块拓展服务
#define         BLE_TEST_CMD                    0x35                            //蓝牙功能性测试（扫描指定蓝牙信标）
#define         GET_VOICE_STATE_CMD             0x60                            //获取语音状态码
#define         MIC_SILENCE_CMD                 0x61                            //MIC静音设置
#define         SET_SPEAKER_VOLUME_CMD          0x62                            //speaker音量设置
#define         VOICE_TEST_CMD                  0x63                            //语音模组音频产测
#define         VOICE_AWAKEN_TEST_CMD           0x64                            //语音模组唤醒产测
#define         VOICE_EXTEND_FUN_CMD            0x65                            //语音模组扩展功能


//=============================================================================
#define MCU_RX_VER              0x00                                            //模块发送帧协议版本号
#define MCU_TX_VER              0x03                                            //MCU 发送帧协议版本号(默认)
#define PROTOCOL_HEAD           0x07                                            //固定协议头长度
#define FRAME_FIRST             0x55                                            //帧头第一字节
#define FRAME_SECOND            0xaa                                            //帧头第二字节
//============================================================================= 

//#define ERROR_NEED_MORE_DATA    1
#define ERROR_SUCCESS           0                                                   // 
#define ERROR_FAILED            -1                                                  //
#define ERROR_SYNC_CODE         -2
#define ERROR_VERSION           -3
#define ERROR_CMD_ID            -4
#define ERROR_PAYLOAD_LEN       -5
#define ERROR_PAYLOAD           -6
#define ERROR_CHECK_SUM         -7
#define ERROR_NEED_MORE_DATA    -8

typedef enum
{
    e_mcu_type_unknown      = -1,
    e_mcu_type_raw          = 0,
    e_mcu_type_bool         = 1,
    e_mcu_type_int          = 2,
    e_mcu_type_string       = 3,
    e_mcu_type_enum         = 4,
    e_mcu_type_bitmap       = 5,

} mcu_type_e;

typedef struct{
    uint8_t         id;
    uint8_t         type;
    uint16_t        data_len;
    uint8_t*        pdata;
} mcu_data_point_t;

typedef struct {
    uint16_t        sync;
    uint8_t         ver;
    uint8_t         cmd;
    uint16_t        payload_len;
    uint8_t*        ppayload;
    uint8_t         check_sum;
    uint8_t*        pdata;
    int             data_len;
} mcu_cmd_frame_t;

int lbmcu_frame_write_buffer(mcu_cmd_frame_t* pmcf);
//int lbmcu_frame_write_buffer(mcu_cmd_frame_t* pmcf, uint8_t* pbuffer, int len);

mcu_cmd_frame_t* lbmcu_open_frame(uint8_t ver, uint8_t cmd_id, const uint8_t* payload, int len);

int lbmcu_get_frame_buffer_size(mcu_cmd_frame_t* pmcf);
//int lbmcu_write_frame(mcu_cmd_frame_t* pmcf);

void lbmcu_close_frame(mcu_cmd_frame_t** ppmcf);

int lbmcu_estimate_frame_size(const uint8_t* pbuf, int size);

int lbmcu_parser_frame(const uint8_t* pbuf, int size, mcu_cmd_frame_t** ppmcf);

// create heart beat frame
mcu_cmd_frame_t* lbmcu_create_heart_beat_frame();

// query product info
