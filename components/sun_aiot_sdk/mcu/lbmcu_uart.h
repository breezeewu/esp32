#pragma once
#include "lbcachebuf.h"
#include "lbmcu_cmd.h"
typedef struct{
    int         uart_num;
    uint8_t*    prx_buffer;
    int         rx_buffer_size;

    struct lbcache_buffer_context*  pcbc;
    mcu_cmd_frame_t**               ppcmd_list;
    int         max_frame_count;
    int         bidx;
    int         eidx;
} mcu_uart_io_t;

void* mcu_uart_recv_proc(struct lbthread_context* ptc);

mcu_uart_io_t* mcu_uart_open(int uart_num, int buf_size);

int mcu_uart_read(mcu_uart_io_t* pmui, uint8_t* pdata, int len);

int mcu_uart_write(mcu_uart_io_t* pmui, const uint8_t* pdata, int len);

int mcu_uart_close(mcu_uart_io_t** ppmui);

int  mcu_uart_read_frame_from_cache(mcu_uart_io_t* pmui);

