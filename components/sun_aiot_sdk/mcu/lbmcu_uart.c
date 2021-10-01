#include "lbmcu_uart.h"
#include <stdio.h>
#include <string.h>
#include "lbmacro.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

void* mcu_uart_recv_proc(struct lbthread_context* ptc)
{
    int ret = 0;
    lbcheck_pointer(ptc, "Invalid parameter, ptc:%p\n", ptc);
    mcu_uart_io_t* pmui = (mcu_uart_io_t*)lbthread_get_owner(ptc);
    lbcheck_pointer(ptc, "Invalid thread owner, pmui:%p\n", pmui);
    //pmui->uart_num = ;
    uart_event_t event;
    size_t buffered_size;
    pmui->prx_buffer = (uint8_t*) malloc(pmui->rx_buffer_size);
    while(lbthread_is_alive(ptc))
    {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            bzero(pmui->prx_buffer, RD_BUF_SIZE);
            //ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                {
                    //ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    while(event.size > 0)
                    {
                        ret = uart_read_bytes(pmui->uart_num, pmui->prx_buffer, event.size, portMAX_DELAY);
                        if(ret <= 0)
                        {
                            lberror("ret:%d = uart_read_bytes(pmui->uart_num:%d, pmui->prx_buffer:%p, event.size:%d, portMAX_DELAY)\n", ret, pmui->uart_num, pmui->prx_buffer, event.size);
                            break;
                        }
                        event.size -= ret;
                        ret = lbcache_buffer_deliver_data(pmui->pcbc, pmui->prx_buffer, ret);
                        lbcheck_break(ret, "ret:%d = lbcache_buffer_deliver_data(pmui->pcbc:%p, pmui->prx_buffer:%p, ret:%d)", ret, pmui->pcbc, pmui->prx_buffer, ret);
                        //assert(0 == ret)
                    };
                    
                    
                    //ESP_LOGI(TAG, "[DATA EVT]:");
                    //uart_write_bytes(pmui->uart_num, (const char*) dtmp, event.size);
                    break;
                }
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    lberror("hw fifo overflow!\n");
                    //ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(pmui->uart_num);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    lberror("ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(pmui->uart_num);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    lberror("uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    lberror("uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    lberror("uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(pmui->uart_num, &buffered_size);
                    int pos = uart_pattern_pop_pos(pmui->uart_num);
                    lberror("[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(pmui->uart_num);
                    } else {
                        uart_read_bytes(pmui->uart_num, pmui->prx_buffer, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(pmui->uart_num, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        lberror("read data: %s", pmui->prx_buffer);
                        lberror("read pat : %s", pat);
                    }
                    break;
                //Others
                default:
                    lberror("uart event type: %d", event.type);
                    break;
            }
        }
    }
    lbfree(pmui->prx_buffer);
    pmui->prx_buffer = NULL;
}

mcu_uart_io_t* mcu_uart_open(int uart_num, int buf_size)
{
    esp_err_t ret = ESP_OK;
    mcu_uart_io_t* pmui = (mcu_uart_io_t*)malloc(sizeof(mcu_uart_io_t));
    memset(pmui, 0, sizeof(mcu_uart_io_t));
    pmui->uart_num = uart_num;
    if(0 == buf_size)
    {
        buf_size = BUF_SIZE;
    }
    pmui->rx_buffer_size = buf_size;
    lbinfo("%s(uart_num:%d) begin\n", __FUNCTION__, uart_num);
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    do{
        ret = uart_driver_install(pmui->uart_num, pmui->rx_buffer_size * 2, pmui->rx_buffer_size * 2, 20, &uart0_queue, 0);
        lbcheck_break(ret, "ret:%d = uart_driver_install(pmui->uart_num:%d, pmui->rx_buffer_size*2:%d, pmui->rx_buffer_size*2:%d, 20, &uart0_queue, 0)\n", ret, pmui->uart_num, pmui->rx_buffer_size * 2, pmui->rx_buffer_size * 2);
        ret = uart_param_config(pmui->uart_num, &uart_config);
        lbcheck_break(ret, "ret:%d = uart_param_config(pmui->uart_num:%d, &uart_config)\n", ret, pmui->uart_num);
        //Set UART pins (using UART0 default pins ie no changes.)
        ret = uart_set_pin(pmui->uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        lbcheck_break(ret, "ret:%d = uart_set_pin\n", ret);
        //Set uart pattern detect function.
        ret = uart_enable_pattern_det_baud_intr(pmui->uart_num, '+', PATTERN_CHR_NUM, 9, 0, 0);
        lbcheck_break(ret, "ret:%d = uart_enable_pattern_det_baud_intr(pmui->uart_num, +, PATTERN_CHR_NUM:%d, 9, 0, 0)\n", ret, PATTERN_CHR_NUM);
        //Reset the pattern queue length to record at most 20 pattern positions.
        ret = uart_pattern_queue_reset(pmui->uart_num, 20);
        lbcheck_break(ret, "ret:%d = uart_pattern_queue_reset(pmui->uart_num, 20)\n");
        lbinfo("%s success, ret:%d, pmui:%p\n", __FUNCTION__, ret, pmui);
        pmui->pcbc = lbcache_buffer_open(pmui->rx_buffer_size * 5);
        pmui->max_frame_count = 20;
        pmui->ppcmd_list = (mcu_cmd_frame_t*)malloc(sizeof(mcu_cmd_frame_t*) * pmui->max_frame_count);
        memset(pmui->ppcmd_list, 0, sizeof(mcu_cmd_frame_t*) * pmui->max_frame_count);
        pmui->bidx = 0;
        pmui->eidx = 0;
        return pmui;
    }while(0);
    //Install UART driver, and get the queue.
    lbfree(pmui);
    lbinfo("%s failed, ret:%d\n", __FUNCTION__, ret);
    return NULL;
}

int mcu_uart_read(mcu_uart_io_t* pmui, uint8_t* pdata, int len)
{
    int read_len = 0;
    if(NULL == pmui || NULL == pdata || len <= 0)
    {
        lberror("Invalid parameter, pmui:%p, pdata:%p, len:%d\n", pmui, pdata, len);
        return -1;
    }
    read_len = uart_read_bytes(pmui->uart_num, pdata, len, portMAX_DELAY);
    lbinfo("read_len:%d = uart_read_bytes(pmui->uart_num:%d, pdata:%p, len:%d, portMAX_DELAY)\n", ret, pmui->uart_num, pdata, read_len);
    
    return read_len;
}

int mcu_uart_write(mcu_uart_io_t* pmui, const uint8_t* pdata, int len)
{
    int wirte_len = 0;
    if(NULL == pmui || NULL == pdata || len <= 0)
    {
        lberror("Invalid parameter, pmui:%p, pdata:%p, len:%d\n", pmui, pdata, len);
        return -1;
    }
    wirte_len = uart_write_bytes(pmui->uart_num, (const char*)pdata, len);
    lbinfo("wirte_len:%d = uart_write_bytes(pmui->uart_num:%d, pdata:%p, len:%d)\n", wirte_len, pmui->uart_num, pdata, len);
    return wirte_len;
}

int mcu_uart_close(mcu_uart_io_t** ppmui)
{
    lbinfo("%s ppmui:%p\n", __FUNCTION__, ppmui);
    if(ppmui && *ppmui)
    {
        mcu_uart_io_t* pmui = *ppmui;
        if(pmui->pcbc)
        {
            lbcache_buffer_closep(&pmui->pcbc);
        }
        if(pmui->ppcmd_list)
        {
            for(; pmui->bidx < pmui->eidx; pmui->bidx = ++pmui->bidx%pmui->max_frame_count)
            {
                lbmcu_close_frame(&pmui->ppcmd_list[pmui->bidx]);
            }
            lbfree(pmui->ppcmd_list);
        }
        uart_driver_delete(pmui->uart_num);
        lbfree(pmui);
    }
    lbinfo("%s end\n", __FUNCTION__);
    //lbinfo("%s ret:%d = uart_driver_delete(pmui->uart_num:%d)\n", __FUNCTION__, ret, pmui->uart_num);
    return ret;
}

int  mcu_uart_read_frame_from_cache(mcu_uart_io_t* pmui)
{
    int ret = 0;
    uint8_t* pbuf = NULL;
    int buf_size = 0;
    mcu_cmd_frame_t* pmcf = NULL;
    lbcheck_pointer(pmui, -1, "Invalid parameter, pmui:%p\n", pmui);

    while(lbcache_buffer_get_remain(pmui->pcbc) >= MCU_CMD_FRAME_HEADER_SIZE){
        buf_size = lbcache_buffer_try_fetch(pmui->pcbc, &pbuf);
        ret = lbmcu_parser_frame(pbuf, buf_size, &pmcf);
        if(ret <= 0)
        {
            lberror("ret:%d = lbmcu_parser_frame(pbuf:%p, buf_size:%d, &pmcf:%p) failed\n", ret, pbuf, buf_size, pmcf);
        }
    };
    pmui->ppcmd_list[pmui-]
}