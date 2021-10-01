#include "lbmacro.h"
#include "lbthread.h"
#include "lbmcu_uart.h"
#include "lbmcu_transport.h"
#include <stdio.h>
#include <string.h>
typedef struct
{
    event_callback      event_cb;
    void*               powner;
    int                 uart_num;
    int                 heart_beat_interval;
    int                 heart_beat_max_response_duration;
    int64_t             llast_update_time;
    int64_t             llast_send_time;

    struct lbthread_context* ptransport_thread;
    mcu_uart_io_t*      pmui;
    uint8_t*            pbufer;
    int                 buffer_size;
} mcu_transport_context_t;

void* mcu_transport_init(int uart_num)
{
    mcu_transport_context_t* pmtc = (mcu_transport_context_t*)malloc(sizeof(mcu_transport_context_t));
    memset(pmtc, 0, sizeof(mcu_transport_context_t));

    pmtc->uart_num = uart_num;
    pmtc->heart_beat_interval_ms = 15000;
    pmtc->heart_beat_max_response_duration = 3000;
    pmtc->pmui =  mcu_uart_open(uart_num, 0);
    pmtc->buffer_size = 256;
    pmtc->pbufer = (uint8_t*)malloc(pmtc->buffer_size);
    memset(pmtc->pbufer, 0, pmtc->buffer_size);
    return pmtc;
}

int mcu_transport_set_heart_beat_interval(void* handle, int interval_ms)
{
    mcu_transport_context_t* pmtc = (mcu_transport_context_t*)handle;
    lbcheck_pointer(handle, -1, "Invalid parameter, handle:%p\n", handle);

    pmtc->heart_beat_interval_ms = interval_ms;
    lbinfo("%s(handle:%p, interval_ms:%d)\n", __FUNCTION__, handle, interval_ms);
    return 0;
}

int mcu_transport_set_event_notify(void* handle, event_callback event_cb, void* powner)
{
    mcu_transport_context_t* pmtc = (mcu_transport_context_t*)handle;
    lbcheck_pointer(handle, -1, "Invalid parameter, handle:%p\n", handle);

    pmtc->event_cb = event_cb;
    pmtc->powner = powner;

    return 0;
}

int mcu_transport_start(void* handle)
{
    int ret = 0;
    mcu_transport_context_t* pmtc = (mcu_transport_context_t*)handle;
    lbcheck_pointer(pmtc, -1, "Invalid parameter, handle:%p\n", pmtc);
    if(pmtc->ptransport_thread)
    {
        lbthread_close(&pmtc->ptransport_thread);
    }

    pmtc->pmui = mcu_uart_open(pmtc->uart_num, 0);
    lbcheck_pointer(pmtc->pmui, -1, "pmtc->pmui:%p = mcu_uart_open(pmtc->uart_num:%d, 0)\n", pmtc->pmui, pmtc->uart_num);
    pmtc->ptransport_thread = lbthread_open("mcu transport", pmtc->pmui, mcu_uart_recv_proc);
    ret = lbthread_start(pmtc->ptransport_thread);
    lbinfo("%s ret:%d = lbthread_start(pmtc->ptransport_thread:%p)\n", __FUNCTION__, ret, pmtc->ptransport_thread);
    return ret;
}

int mcu_transport_stop(void* handle)
{
    int ret = 0;
    mcu_transport_context_t* pmtc = (mcu_transport_context_t*)handle;
    lbcheck_pointer(pmtc, -1, "Invalid parameter, handle:%p\n", pmtc);
    if(pmtc->ptransport_thread)
    {
        ret = lbthread_stop(pmtc->ptransport_thread);
        lbthread_close(&pmtc->ptransport_thread);
    }
    lbinfo("%s ret:%d = lbthread_stop\n", __FUNCTION__, ret);
    return ret;
}

void mcu_transport_deinit(void** pphandle)
{
    lbinfo("%s(pphandle:%p)\n", __FUNCTION__, pphandle);
    if(pphandle && *pphandle)
    {
        mcu_transport_context_t* pmtc = *pphandle;
        lbfree(pmtc->pbufer);
        lbfree(pmtc);
        *pphandle = NULL;
    }

    lbinfo("%s end\n", __FUNCTION__);
}

int mcu_transport_write_frame(void* handle, mcu_cmd_frame_t* pmcf)
{
    int ret = 0;
    mcu_transport_context_t* pmtc = (mcu_transport_context_t*)handle;
    if(NULL == pmtc || NULL == pmcf || NULL == pmtc->pmui)
    {
        lberror("Invalid parameter, pmtc:%p, pmcf:%p, pmtc->pmui:%p\n", pmtc, pmcf, pmtc ? pmtc->pmui : NULL);
        return -1;
    }

    //int frame_size = lbmcu_get_frame_buffer_size(pmcf);
    //uint8_t* pbuf = (uint8_t*)malloc(frame_size);
    ret = lbmcu_frame_write_buffer(pmcf);
    if(ret < MCU_CMD_FRAME_HEADER_SIZE)
    {
        lberror("ret:%d = lbmcu_frame_write_buffer(pmcf:%p, pmtc->pbufer:%p, pmtc->buffer_size:%d)\n", ret, pmcf, pmtc->pbufer, pmtc->buffer_size);
        return -1;
    }

    return 0;
}

mcu_cmd_frame_t* mcu_transport_read_frame(void* handle)
{

}

int on_recv_mcu_frame(void* handle, mcu_cmd_frame_t* pmcf)
{
    
}