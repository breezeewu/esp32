#include <stdio.h>
#include <string.h>
#include "lbmcu_cmd.h"
#include "lbmacro.h"
const int mcu_sync_word = 0x55aa;
int int_to_byte(unsigned long number,unsigned char* pbuf, int len)
{
    int i = 0;
    lbcheck_pointer(pbuf, -1, "Invalid parameter, pbuf:%p\n", pbuf);

    for(i = 0; i < len; i++)
    {
        pbuf[i] = number >> (len -1)*8;
    }
    
    return 0;
}

unsigned long byte_to_int(const uint8_t* pbuf, int len)
{
    int i = 0;
    unsigned long nubmer = 0;

    for(i = 0; i < len && i < sizeof(nubmer); i++)
    {
        nubmer <<= 8;
        nubmer += pbuf[i] & 0xff;
    }
    
    return nubmer;
}

int lbmcu_frame_write_buffer(mcu_cmd_frame_t* pmcf)
{
    int offset = 0;
    int sum = 0;
    int i = 0;
    lbcheck_pointer(pmcf, -1, "Invalid parameter, pmcf:%p\n", pmcf);

    pmcf->payload_len = pmcf->payload_len + MCU_CMD_FRAME_HEADER_SIZE;
    pmcf->ppayload = malloc(pmcf->payload_len);
    int_to_byte(pmcf->sync, pmcf->pdata + offset, 2);
    offset += 2;

    pmcf->pdata[offset++] = pmcf->ver;
    pmcf->pdata[offset++] = pmcf->cmd;

    int_to_byte(pmcf->payload_len, pmcf->pdata + offset, 2);
    offset += 2;

    memcpy(pmcf->pdata + offset, pmcf->ppayload, pmcf->payload_len);
    offset += pmcf->payload_len;

    for(i = 0; i < offset; i++)
    {pbuf[offset]
        sum += pmcf->pdata[i];
    }
    pmcf->check_sum = sum%256;
    pmcf->pdata[offset++] = pmcf->check_sum;

    return offset;
}

mcu_cmd_frame_t* lbmcu_open_frame(uint8_t ver, uint8_t cmd_id, const uint8_t* payload, int len)
{
    mcu_cmd_frame_t* pmcf = (mcu_cmd_frame_t*)malloc(sizeof(mcu_cmd_frame_t));
    meset(pmcf, 0, sizeof(mcu_cmd_frame_t));
    pmcf->sync = mcu_sync_word;
    pmcf->ver = ver;
    pmcf->cmd = cmd_id;
    pmcf->payload_len = len;
    pmcf->ppayload = (void*)malloc(len);
    memcpy(pmcf->ppayload, payload, pmcf->payload_len);
    //lbmcu_frame_write_buffer(pmcf);

    return pmcf;
}

int lbmcu_get_frame_buffer_size(mcu_cmd_frame_t* pmcf)
{
    lbcheck_pointer(pmcf, "Invalid parameter, pmcf:%p\n", pmcf);

    return pmcf->payload_len + MCU_CMD_FRAME_HEADER_SIZE;
}

void lbmcu_close_frame(mcu_cmd_frame_t** ppmcf)
{
    if(ppmcf && *ppmcf)
    {
        mcu_cmd_frame_t* pmcf = *ppmcf;
        lbfree(pmcf->ppayload);
        lbfree(pmcf->pdata);
        lbfree(pmcf);
        *ppmcf = pmcf;
    }
}

int lbmcu_parser_frame(const uint8_t* pbuf, int size, mcu_cmd_frame_t** ppmcf)
{
    int ret = ERROR_FAILED;
    int offset = 0;
    int sum = 0;
    int cmd_size = 0;
    lbinfo("%s(pbuf:%p, size:%d, ppmcf:%p)\n", __FUNCTION__, pbuf, size, ppmcf);
    if(NULL == pbuf || NULL == ppmcf)
    {
        lberror("Invalid parameter, pbuf:%p, ppmcf:%p\n", pbuf, ppmcf);
        return ERROR_FAILED;
    }

    if(size < MCU_CMD_FRAME_HEADER_SIZE)
    {
        lberror("need more data, size:%d < MCU_CMD_FRAME_HEADER_SIZE(7)\n", size);
        return ERROR_NEED_MORE_DATA;
    }

    mcu_cmd_frame_t* pmcf = (mcu_cmd_frame_t*)calloc(1, sizeof(mcu_cmd_frame_t));//lbzmalloc(1, mcu_cmd_frame_t);
    do{
        pmcf->sync = (pbuf[offset++] << 8) | (pbuf[offset++] & 0xff);
        lbcheck_value_break(mcu_sync_word != pmcf->sync, "Invalid sync header, sync:%x != mcu_sync_word:%x", pmcf->sync, mcu_sync_word);
        pmcf->ver = pbuf[offset++];
        pmcf->cmd = pbuf[offset++];
        pmcf->payload_len = byte_to_int(pbuf + offset, 2);
        offset += 2;
        if(pmcf->payload_len + MCU_CMD_FRAME_HEADER_SIZE > size)
        {
            lberror("%s no enought buffer for mcu frame parser, need:%d, have:%d\n", __FUNCTION__, pmcf->payload_len + MCU_CMD_FRAME_HEADER_SIZE, size);
            ret = ERROR_FAILED;
            break;
        }
        offset += pmcf->payload_len;
        for(i = 0; i < offset; i++)
        {
            sum += 0xff & pbuf[i];
        }

        if(pbuf[offset] != sum % 256)
        {
            lberror("%s check sum invalid, current:%x, now:%x, offset:%d\n", __FUNCTION__, pbuf[offset], sum%256, offset);
            ret = ERROR_CHECK_SUM;
            break;
        }
        *ppmcf = pmcf;
        lbinfo("%s success, pmcf:%p\n", __FUNCTION__, pmcf);
        return offset;
    }while(0);
    
    lbmcu_close_frame(&pmcf);
    lberror("%s failed\n", __FUNCTION__);
    lbmemory(3, pbuf, offset, "Invalid mcu cmd memory:");
    return ret;
}

mcu_cmd_frame_t* lbmcu_create_heart_beat_frame()
{
    mcu_cmd_frame_t* pmcf = lbmcu_open_frame(HEAT_BEAT_CMD, MCU_RX_VER, NULL, 0);

    return pmcf;
}