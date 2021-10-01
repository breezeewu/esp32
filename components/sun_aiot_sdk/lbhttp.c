#include "lbhttp.h"
#include <stdio.h>
#include "lburl.h"
#include "lbnet.h"
#include "http_parser.h"
#include "lbstring.h"
#include "lbthread.h"
#include "lbkv_list.h"
#include "lbmbedtls.h"

lbhttp_context* lbhttp_open(const char* purl)
{
    lbhttp_context* phc = NULL;
    lbcheck_pointer(purl, NULL, "%s invalid parameter, purl:%p\n", purl);
    phc = (lbhttp_context*)lbmalloc(sizeof(lbhttp_context));
    memset(phc, 0, sizeof(lbhttp_context));

    phc->request.puc = lburl_context_open(purl);
    phc->pnc = lbnet_open(e_net_type_mbed_tls);
    lbstrcp(phc->request.pmethod, "POST");
    lbinfo("phc->request.puc:%p = lburl_context_open(purl:%s), phc->pnc:%p\n", phc->request.puc, purl, phc->pnc);
    //lbstrcp(phc->pmethod, "POST");
    return phc;
}

int lbhttp_set_string_opt(lbhttp_context* phc, e_aiot_http_option opt, const char* pval)
{
    if(NULL == phc || NULL == pval)
    {
        lberror("%s invalid parameter, phc:%p, pval:%s\n", __FUNCTION__, phc, pval);
        return -1;
    }

    switch(opt)
    {
        case e_aiot_http_method:
        {
            lbfree(phc->request.pmethod);
            lbstrcp(phc->request.pmethod, pval);
            break;
        }
        case e_aiot_http_host:
        {
            lbfree(phc->request.phost);
            lbstrcp(phc->request.phost, pval);
            break;
        }
        case e_aiot_http_content_form:
        {
            lbfree(phc->request.pcontent_type);
            lbstrcp(phc->request.pcontent_type, pval);
            break;
        }
        case e_aiot_http_content_body:
        {
            lbfree(phc->request.pcontent_body);
            lbstrcp(phc->request.pcontent_body, pval);
            phc->request.econtent_type = e_aiot_http_content_string;
            break;
        }
        default:
        {
            lberror("%s invalid option:%d, pval:%s\n", __FUNCTION__, opt, pval);
            return -1;
        }
    }
    
    return 0;
}

int lbhttp_set_long_opt(lbhttp_context* phc, e_aiot_http_option opt, long lval)
{
    if(e_aiot_http_content_length == opt)
    {
        phc->request.lcontent_length = lval;
        return 0;
    }

    return -1;
}

int lbhttp_set_request_content(lbhttp_context* phc, e_aiot_http_content_type con_type, const char* pval, int content_len)
{
    if(NULL == phc || NULL == pval)
    {
        lberror("%s invalid parameter, phc:%p, pval:%s\n", __FUNCTION__, phc, pval);
        return -1;
    }
    lbinfo("%s begin\n", __FUNCTION__);
    lbfree(phc->request.pcontent_body);
    if(e_aiot_http_content_octet == con_type)
    {
        if(content_len <= 0)
        {
            lberror("%s invalid parameter, con_type:%d, content_len:%d\n", __FUNCTION__, con_type, content_len);
            return -1;
        }
        phc->request.pcontent_body = (char*)lbmalloc(content_len);
        memcpy(phc->request.pcontent_body, pval, content_len);
        //phc->request.lcontent_length = content_len;
    }
    else
    {
        lbstrcp(phc->request.pcontent_body, pval); 
    }
    phc->request.lcontent_length = content_len;
    phc->request.econtent_type = con_type;
    lbinfo("%s end\n", __FUNCTION__);
    return 0;
}

int lbhttp_set_response_content_save_path(lbhttp_context* phc, const char* tmp_path, const char* dst_path, on_http_msg_cb download_cb, void* powner)
{
    if(NULL == phc || NULL == dst_path)
    {
        lberror("%s invalid parameter, phc:%p, tmp_path:%s, dst_path:%s\n", __FUNCTION__, phc, tmp_path, dst_path);
        return -1;
    }
    phc->download_cb = download_cb;
    phc->powner = powner;
    phc->response.con_type = e_aiot_http_content_file;
    lbstrcp(phc->response.ptmp_path, tmp_path);
    lbstrcp(phc->response.pdst_path, dst_path);


    return 0;
}

int lbhttp_send_header(lbhttp_context* phc)
{
    lbhttp_request_context* req =  NULL;
    char* buffer = NULL;
    int ret = 0;
    if(NULL == phc || NULL == phc->pnc|| NULL == phc->request.puc)
    {
        lberror("%s invalid parameter, phc:%p, phc->request.puc:%p, phc->pnc:%p\n", __FUNCTION__, phc, phc ? phc->request.puc : NULL, phc ? phc->pnc : NULL);
        return -1;
    }
    lbinfo("lbhttp_send_header(phc:%p)\n", phc);
    req = &phc->request;
    lbinfo("before lbnet_connect(phc->pnc:%p, req->puc->phost:%s, req->puc->port:%d)\n", phc->pnc, req->puc->phost, req->puc->port);

    /*void* handle = lbnet_mbed_tls_conenct_test(NULL, req->puc->phost, req->puc->port);
    lbinfo("handle:%p = lbnet_mbed_tls_conenct_test(host:%s, %d)\n", handle, req->puc->phost, req->puc->port);
    lbnet_mbed_tls_close(handle);
    return -1;*/
    buffer = (char*)lbmalloc(2048);
    memset(buffer, 0, 2048);
    do{
        ret = lbnet_connect(phc->pnc, req->puc->phost, req->puc->port);
        //lbinfo("ret:%d = lbnet_connect(phc->pnc, req->puc->phost, req->puc->port)\n", ret);

        //phc->pnc = lbtcp_open(req->puc->phost, req->puc->port, true);
        if(0 != ret)
        {
            http_msg_callback(phc, e_aiot_http_msg_on_connect_error, (long)strerror(errno), errno);
            lberror("ret:%d = lbnet_connect(phc->pnc:%p, req->puc->phost:%s, req->puc->port:%d) failed\n", ret, phc->pnc, req->puc->phost, req->puc->port);
            break;
        }

        http_msg_callback(phc, e_aiot_http_msg_on_connect_complete, 0, 0);
        sprintf(buffer, "%s %s", req->pmethod, req->puc->ppath);
        if(req->puc->pparam)
        {
            sprintf(buffer + strlen(buffer), "?%s", req->puc->pparam);
        }
        lbinfo("buffer1:%s\n", buffer);
        sprintf(buffer + strlen(buffer), " %s\r\n", HTTP_VERSION);
        if(req->puc->phost)
        {
            sprintf(buffer + strlen(buffer), "Host: %s\r\n", req->puc->phost);
        }
        lbinfo("buffer2:%s\n", buffer);
        sprintf(buffer + strlen(buffer), "%s", "Connection: Keep-Alive\r\nUser-Agent: sunvalley device SDK V2021.06.25\r\n");
        lbinfo("buffer3:%s\n", buffer);
        /*sprintf(buffer, "%s %s %s\r\nHost: %s\r\nConnection: Keep-Alive\r\nUser-Agent: sunvalley device SDK V2021.06.25\r\n", 
        req->pmethod, req->puc->ppath, HTTP_VERSION, req->puc->phost);*/
        if(e_aiot_http_conent_none != req->econtent_type)
        {
            if(e_aiot_http_content_file == req->econtent_type)
            {
                req->pfile = fopen(req->pcontent_body, "rb");
                if(req->pfile)
                {
                    fseek(req->pfile, 0, SEEK_END);
                    req->lcontent_length = ftell(req->pfile);
                    fseek(req->pfile, 0, SEEK_SET);
                    //fclose(req->pfile);
                }
            }
            sprintf(buffer + strlen(buffer), "Content-Length: %ld\r\nContent-Type: %s\r\n\r\n", req->lcontent_length, req->pcontent_type ? req->pcontent_type : HTTP_CONTENT_TYPE_JSON);
        }
        lbinfo("buffer4:%s\n", buffer);
        //lbinfo("after http_msg_callback\n");
        //return -1;
        ret = lbnet_write(phc->pnc, buffer, strlen(buffer));
        //lbinfo("ret:%d = lbnet_write(phc->pnc, buffer:%s, strlen(buffer):%d)\n", ret, buffer, strlen(buffer));
        if(ret < strlen(buffer))
        {
            http_msg_callback(phc, e_aiot_http_msg_on_request_error, (long)strerror(errno), errno);
            lberror("ret:%d = lbnet_write(phc->pnc, buffer, strlen(buffer):%d)\n", ret, strlen(buffer));
        }
        else
        {
            http_msg_callback(phc, e_aiot_http_msg_on_request_header_complete, 0, 0);
        }
    }while(0);

    ret = ret >= strlen(buffer) ? 0 : -1;
    lbfree(buffer);
    lbinfo("%s end, ret:%d\n", __FUNCTION__, ret);
    return ret;
}

int lbhttp_send_content(lbhttp_context* phc)
{
    int ret = -1;
    size_t read_len = 0;
    size_t send_len = 0;
    char* buf = NULL;
    
    lbcheck_pointer(phc, -1, "Invalid parameter:phc:%p\n", phc);
    lbinfo("%s(phc:%p), phc->request.econtent_type:%d, phc->request.lcontent_length:%ld\n", __FUNCTION__, phc, phc->request.econtent_type, phc->request.lcontent_length);
    if(e_aiot_http_content_file == phc->request.econtent_type)
    {
        buf = (char*)lbmalloc(1024);
        phc->request.lsend_bytes = 0;
        lbcheck_pointer(phc->request.pfile, -1, "no request octet file found, phc->request.pfile:%p\n", phc->request.pfile);
        do{
            read_len = fread(buf, 1 , 1024, phc->request.pfile);
            lbcheck_value_break(read_len <= 0, "read_len:%d = fread(buffer, 1 , 1024, phc->request.pfile:%p) failed\n", read_len, phc->request.pfile);
            /*if(read_len <= 0)
            {
                lberror("read_len:%ld = fread(buffer, 1 , 1024, phc->request.pfile:%p)\n", read_len, phc->request.pfile);
                break;
            }*/
            send_len = lbnet_write(phc->pnc, buf, read_len);
            lbcheck_value_break(send_len < read_len, "send_len:%ld = lbnet_write(phc->pnc, buffer, read_len:%ld) failed\n", send_len, read_len, phc->request.pfile);
            /*if(send_len < read_len)
            {
                lberror("send_len:%ld = lbnet_write(phc->pnc, buffer, read_len:%ld) failed\n", send_len, read_len);
                break;
            }*/
            phc->request.lsend_bytes += send_len;
        }while(phc->request.lsend_bytes < phc->request.lcontent_length);
    }
    else if(phc->request.pcontent_body && phc->request.lcontent_length > 0)
    {
        send_len = lbnet_write(phc->pnc, phc->request.pcontent_body, phc->request.lcontent_length);
        if(send_len < phc->request.lcontent_length)
        {
            lberror("send_len:%ld = lbnet_write(phc->pnc, phc->request.pcontent_body, phc->request.lcontent_length:%ld) failed\n", send_len, phc->request.lcontent_length);
            //http_msg_callback(phc, HTTP_PROGRESS_SEND_REQUEST_ERROR, strerror(errno), errno);
        }
        else
        {
            phc->request.lsend_bytes += send_len;
        }
        
        //lbcheck_value(send_len < read_len, -1, "send_len:%ld = lbnet_write(phc->pnc, phc->request.pcontent_body, phc->request.lcontent_length:%ld) failed\n", send_len, phc->request.lcontent_length);
         
        //lbinfo("send_len:%d = lbnet_write(phc->pnc:%p, phc->request.pcontent_body:%s, phc->request.lcontent_length:%d)\n", send_len, phc->pnc, phc->request.pcontent_body, phc->request.lcontent_length);
    }
    send_len = lbnet_write(phc->pnc, LBSP_HTTP_CRLF, strlen(LBSP_HTTP_CRLF));
    //lbinfo("send_len:%d = lbnet_write(phc->pnc, LBSP_HTTP_CRLF, strlen(LBSP_HTTP_CRLF))\n", send_len);
    if(phc->request.lsend_bytes >= phc->request.lcontent_length)
    {
        http_msg_callback(phc, e_aiot_http_msg_on_request_complete, 0, 0);
        ret = 0;
    }
    else
    {
        http_msg_callback(phc, e_aiot_http_msg_on_request_error, (long)strerror(errno), errno);
        ret = -1;
    }

    lbfree(buf);
    lbinfo("%s end, ret:%d\n", __FUNCTION__, ret);
    return ret;
}

int lbhttp_send_request(lbhttp_context* phc)
{
    int ret = lbhttp_send_header(phc);
    lbcheck_return(ret, "ret:%d = lbhttp_send_header(phc:%p) failed\n", ret, phc);
    ret = lbhttp_send_content(phc);
    lbcheck_return(ret, "ret:%d = lbhttp_send_content(phc:%p) failed\n", ret, phc);

    return ret;
}

int lbhttp_parser_response_header(lbhttp_context* phc, const char* pbuf, int len)
{
    char line[256];
    const char* pnext = NULL;
    char* splt = NULL;
    do{
        memset(line, 0, 256);
        pnext = lbstring_read_line(pbuf, line, 256);
        //lbinfo("pnext:%s = lbstring_read_line(pbuf:%s, line:%s, 256)\n", pnext, pbuf, line);
        if(pnext && strlen(line) <= 0)
        {
            break;
        }
        splt = strchr(line, ':');
        if(splt)
        {
            *splt = '\0';
            splt++;
            if(*splt == ' ')
            {
                splt++;
            }
            lbkv_list_add(phc->response.pkv_list, line, splt);
            //lbinfo("%s lbkv_list_add(phc->response.pkv_list, line, splt)\n", __FUNCTION__, phc->response.pkv_list, line, splt);
        }
        pbuf = pnext;
    }
    while(pnext);

    return 0;
}

int http_recv_response(lbhttp_context* phc)
{
    int ret = -1;
    char* line = NULL;
    char* splt = NULL;
    char* pbuf = NULL;
    const char* pnext = NULL;
    bool header_complete = false;
    int read_len = 0;
    int offset = 0;
    long last_record_time = 0;
    long last_record_download_bytes = 0;
    unsigned long begin_time = lbget_sys_time();
    int percent = -1;
    lbinfo("%s(phc:%p)\n", __FUNCTION__, phc);
    if(NULL == phc)
    {
        lberror("%s invalid parameter, phc:%p\n", __FUNCTION__, phc);
        return -1;
    }

    phc->response.pkv_list = lbkv_list_open();
    //lbinfo("phc->response.pkv_list:%p = lbkv_list_open(), phc->response.con_type:%d, phc->response.ptmp_path:%p\n", phc->response.pkv_list, phc->response.con_type, phc->response.ptmp_path);
    if(e_aiot_http_content_file == phc->response.con_type)
    {
        phc->response.pfile = fopen(phc->response.ptmp_path, "wb");
        lbinfo("phc->response.pfile:%p = fopen(phc->response.ptmp_path:%s, wb)\n", phc->response.pfile, phc->response.ptmp_path);
        if(NULL == phc->response.pfile)
        {
            lberror("%s create %s file failed\n", __FUNCTION__, phc->response.ptmp_path);
            return -1;
        }
    }
    //lbinfo("before lbmalloc\n");
    line = (char*)lbmalloc(256);
    //lbinfo("line:%p\n", line);
    pbuf = (char*)lbmalloc(4096);
    memset(pbuf, 0, 4096);
    //lbinfo("pbuf:%p\n", pbuf);
    pnext = pbuf;
    //lbinfo("%s before while\n", __FUNCTION__);
    //return -1;
    do{
        //lbinfo("do begin\n");
        if(phc->phttp_thread && !lbthread_is_alive(phc->phttp_thread))
        {
            lbtrace("thread stop, http download break!\n");
            break;
        }

        if(header_complete && phc->hdp.ltotal_bytes <= phc->hdp.ldownload_bytes)
        {
            lbinfo("%s read http content complete, phc->hdp.ltotal_bytes:%ld <= phc->hdp.ldownload_bytes:%ld\n", __FUNCTION__, phc->hdp.ltotal_bytes, phc->hdp.ldownload_bytes);
            break;
        }

        read_len = lbnet_read(phc->pnc, pbuf, 4096);
        //lbinfo("read_len:%d = lbnet_read(phc->pnc:%p, buffer:%s, 1500)\n", read_len, phc->pnc, pbuf);
        if(read_len <= 0)
        {
            ret = read_len;
            lberror("read_len:%d = lbnet_read(phc->pnc, buffer, 4096)\n", read_len);
            break;
        }

        if(header_complete)
        {
            if(phc->response.pfile)
            {
                ret = fwrite(pbuf, 1, read_len, phc->response.pfile);
                if(ret < read_len)
                {
                    lberror("ret:%d = fwrite(pbuf, 1, read_len, phc->response.pfile) failed\n", ret);
                    break;
                }
                phc->hdp.ldownload_bytes = ftell(phc->response.pfile);
                //lbinfo("http download progreess,phc:%p, phc->response.powner:%p, lspeed_bits_per_sec:%ld,lrecv_bytes:%ld, lcontent_length:%ld, lspend_time_ms:%ld\n", phc, phc->response.powner, phc->response.lspeed_bits_per_sec, phc->response.lrecv_bytes, phc->response.lcontent_length, phc->response.lspend_time_ms);
            }
            else
            {
                if(e_aiot_http_content_file == phc->response.con_type)
                {
                    memcpy(phc->response.presponse + phc->hdp.ldownload_bytes, pbuf, read_len);
                    phc->hdp.ldownload_bytes += read_len;
                }
                
                //lbinfo("memcpy\n");
            }

            phc->hdp.lspend_time_ms = lbget_sys_time() - begin_time;
            percent = phc->hdp.ldownload_bytes * 100 / phc->hdp.ltotal_bytes;

            if(phc->hdp.lspend_time_ms - last_record_time >= 1000)
            {
                phc->hdp.ldownload_speed = (phc->hdp.ldownload_bytes - last_record_download_bytes) * 8 * 1000 / (phc->hdp.lspend_time_ms - last_record_time);
                //lbinfo("phc->hdp.ldownload_speed:%ld = (phc->hdp.ldownload_bytes - last_record_download_bytes):%ld * 8 * 1000 / (phc->hdp.lspend_time_ms - last_record_time):%ld\n", phc->hdp.ldownload_speed, (phc->hdp.ldownload_bytes - last_record_download_bytes), (phc->hdp.lspend_time_ms - last_record_time));
                last_record_download_bytes = phc->hdp.ldownload_bytes;
                last_record_time = phc->hdp.lspend_time_ms;
            }
            if(percent != phc->lpercent)
            {
                phc->lpercent = percent;
                http_msg_callback(phc, e_aiot_http_msg_on_response_body_progress, (long)&phc->hdp, phc->lpercent);
            }

            continue;
        }
        //lbinfo("before while\n");
        while(!header_complete && offset < read_len)
        {
            memset(line, 0, 256);
            pnext = lbstring_read_line(pnext, line, 256);
            //lbinfo("read line:%s\n", line);
            //lbinfo("pnext:%s = lbstring_read_line(pnext, line:%s, 256)\n", pnext, line);
            if(memcmp(HTTP_VERSION, line, strlen(HTTP_VERSION)) == 0)
            {
                char tag[256];
                // read http version tag
                const char* ptmp = lbstring_read_tag(line, " ", tag, 256);
                ptmp = lbstring_read_tag(ptmp, " ", tag, 256);
                phc->response.nhttpCode = atoi(tag);
                ptmp = lbstring_read_tag(ptmp, " ", tag, 256);
                phc->response.pstateMsg = (char*)lbmalloc(strlen(tag) + 1);
                memcpy(phc->response.pstateMsg, tag, strlen(tag) + 1);
                //lbinfo("recv http header\n");
                continue;
            }
            else if(pnext && strlen(line) <= 0)
            {
                //lbinfo("recv resp body\n");
                header_complete = true;
                offset = pnext - pbuf;
                phc->response.lcontent_length = 0;
                lbkv_list_get_long_value(phc->response.pkv_list, "Content-Length", &phc->response.lcontent_length);
                phc->hdp.ltotal_bytes = phc->response.lcontent_length;
                phc->hdp.lspend_time_ms = lbget_sys_time() - begin_time;
                if(phc->response.pfile && phc->response.lcontent_length > 0)
                {
                    fwrite(pnext, 1, read_len - offset, phc->response.pfile);
                    phc->hdp.ldownload_bytes = ftell(phc->response.pfile);
                }
                else
                {
                    //lbinfo("copy resp, phc->response.lcontent_length:%d\n", phc->response.lcontent_length);
                    lbfree(phc->response.presponse);
                    //lbinfo("lbfree, read_len - offset:%d\n", read_len - offset);
                    if(phc->response.lcontent_length > 0)
                    {
                        phc->response.presponse = (char*)lbmalloc(phc->response.lcontent_length);
                        //lbinfo("phc->response.presponse:%p = lbmalloc\n", phc->response.presponse);
                        memcpy(phc->response.presponse, pnext, read_len - offset);
                    }
                    
                    phc->hdp.ldownload_bytes = read_len - offset;
                    //lbinfo("phc->response.lcontent_length:%ld, phc->response.presponse:%p\n", phc->response.lcontent_length, phc->response.presponse);
                }
                http_msg_callback(phc, e_aiot_http_msg_on_response_header_complete, 0, 0);
                //lbinfo("after http_msg_callback\n");
                break;
            }
            splt = strchr(line, ':');
            if(splt)
            {
                *splt = '\0';
                splt++;
                if(*splt == ' ')
                {
                    splt++;
                }
                lbkv_list_add(phc->response.pkv_list, line, splt);
                //lbinfo("%s lbkv_list_add(phc->response.pkv_list, line, splt)\n", __FUNCTION__, phc->response.pkv_list, line, splt);
            }
            offset = pnext - pbuf;
            //usleep(200000);
        };
        //lbinfo("while end, pnext:%p\n", pnext);
    }while(pnext);
    //lbinfo("%s after while\n", __FUNCTION__);
    if(phc->hdp.ltotal_bytes <= phc->hdp.ldownload_bytes)
    //if(phc->hpi.ltotal_bytes <=  phc->response.lrecv_bytes)
    {
        if(phc->response.pfile)
        {
            lbfclosep(phc->response.pfile);
            rename(phc->response.ptmp_path, phc->response.pdst_path);
        }
        phc->lpercent = 100;
        http_msg_callback(phc, e_aiot_http_msg_on_response_complete, &phc->hdp, phc->lpercent);
        //http_msg_callback(phc, HTTP_PROGRESS_RECV_RESPONSE_COMPLETE, &phc->hpi, &phc->hpi.lpercent);
        ret = 0;
    }
    else
    {
        http_msg_callback(phc, e_aiot_http_msg_on_response_error, (long)strerror(errno), errno);
        //http_msg_callback(phc, HTTP_PROGRESS_RECV_RESPONSE_ERROR, strerror(errno), errno);
        lberror("http response recv error, need:%ld, recv:%ld\n", phc->response.lcontent_length,  phc->hdp.ldownload_bytes);
        if(phc->response.pfile)
        {
            lbfclosep(phc->response.pfile);
            remove(phc->response.ptmp_path);
        }
        ret = -1;
    }

    lbfree(line);
    lbfree(pbuf);
    lbinfo("%s end, ret:%d\n", __FUNCTION__, ret);
    return ret;
}

void lbhttp_close(lbhttp_context** pphc)
{
    if(pphc && *pphc)
    {
        lbhttp_context* phc = *pphc;
        //lbinfo("%s begin\n", __FUNCTION__);
        // destroy http request context
        lburl_context_close(&phc->request.puc);
        lbfree(phc->request.pmethod);
        lbfree(phc->request.phost);
        lbfree(phc->request.pconnection);
        lbfree(phc->request.pcontent_type);
        lbfree(phc->request.pcontent_body);
        lbfclosep(phc->request.pfile);

        // destroy http response context
        lbfree(phc->response.pstateMsg);
        lbfree(phc->response.presponse);
        lbfclosep(phc->response.pfile);
        lbkv_list_close(&phc->response.pkv_list);

        // destroy http context
        lbthread_close(&phc->phttp_thread);
        lbnet_close(&phc->pnc);
        lbfree(phc);

        *pphc = NULL;
        //lbinfo("%s end\n", __FUNCTION__);
    }
}

void* http_proc(void* arg)
{
    int ret = 0;
    struct lbthread_context* ptc = (struct lbthread_context*)arg;
    lbhttp_context* phc = lbthread_get_owner(ptc);
    lbcheck_pointer(ptc, NULL, "Invalid parameter, ptc:%p\n", ptc);
    lbcheck_pointer(phc, NULL, "Invalid parameter, phc:%p\n", phc);
    //lbinfo("http_proc begin\n");
    //if(lbthread_is_alive(phc))
    {
        ret = lbhttp_send_request(phc);
        lbcheck_value(ret!=0, NULL, "ret:%d = lbhttp_send_request(phc:%p) failed\n", ret, phc);
        //lbinfo("ret:%d = lbhttp_send_request(phc:%p)\n", ret, phc);
        ret = http_recv_response(phc);
        lbcheck_value(ret!=0, NULL, "ret:%d = http_recv_response(phc:%p) failed\n", ret, phc);
        //lbinfo("ret:%d = http_recv_response(phc:%p)\n", ret, phc);
    }

    //lbinfo("http_proc end\n");
    return NULL;
}

int lbhttp_thread_start(lbhttp_context* phc)
{
    int ret = 0;
    assert(phc);
    phc->phttp_thread = lbthread_open("http", phc, http_proc);
    lbcheck_pointer(phc->phttp_thread, -1, "phc->phttp_thread:%p = lbthread_open(http, phc:%p, http_proc:%p) failed\n", phc->phttp_thread, phc, http_proc);
    ret = lbthread_start(phc->phttp_thread);
    //lbinfo("ret:%d = lbthread_start(phc->phttp_thread:%p, false)\n", ret, phc->phttp_thread);
    return ret;
}

int lbhttp_thread_stop(lbhttp_context* phc)
{
    int ret = 0;
    lbcheck_pointer(phc, -1, "Invalid parameter, phc:%p\n", phc);
    lbcheck_pointer(phc->phttp_thread, -1, "Invalid parameter, phc->phttp_thread:%p\n", phc->phttp_thread);

    ret = lbthread_stop(phc->phttp_thread);
    lbthread_close(&phc->phttp_thread);

    return ret;
}

int lbhttp_post_request(const char* purl, const char* preq, int* phttp_code, char* resp, int resp_len)
{
    int ret = -1;
    lbinfo("%s begin\n", __FUNCTION__);
    lbhttp_context* phc = lbhttp_open(purl);
    lbcheck_pointer(phc, ret, "phc:%p = lbhttp_open(purl:%s) failed\n", phc, purl);
    do{
        ret = lbhttp_set_request_content(phc, e_aiot_http_content_string, preq, strlen(preq));
        lbcheck_break(ret, "ret:%d = lbhttp_set_request_content(phc:%p, e_aiot_http_content_string, preq:%s, strlen(preq)) failed\n", ret, phc, preq);
        ret = lbhttp_send_request(phc);
        lbcheck_break(ret, "ret:%d = lbhttp_send_request(phc:%p) failed\n", ret, phc);
        ret = http_recv_response(phc);
        lbcheck_break(ret, "ret:%d = http_recv_response(phc:%p) failed\n", ret, phc);
    }while(0);
    //lbinfo("ret:%d, resp:%p, resp_len:%d, phc->response.presponse:%p, phc->response.lcontent_length:%d\n", ret, resp, resp_len, phc->response.presponse, phc->response.lcontent_length);
    if(0 == ret && NULL != resp && NULL != phc->response.presponse && resp_len >= phc->response.lcontent_length)
    {
        memcpy(resp, phc->response.presponse, phc->response.lcontent_length);
        ret = phc->response.lcontent_length;
        //lbinfo("resp:%s\n", resp);
    }
    
    if(phttp_code)
    {
        *phttp_code = phc->response.nhttpCode;
    }

    lbhttp_close(&phc);
    lbinfo("%s end, ret:%d\n", __FUNCTION__, ret);
    return ret;
}

int http_msg_callback(lbhttp_context* phc, int status, long wparam, long lparam)
{
    int ret = 0;
    lbcheck_pointer(phc, -1, "Invalid param, phc:%p\n", phc);

    phc->msg_type = status;
    if(phc->download_cb)
    {
        phc->download_cb(phc, phc->powner, status, wparam, lparam);
    }

    return ret;
}