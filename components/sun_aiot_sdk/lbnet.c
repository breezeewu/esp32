#include "lbmacro.h"
#include "lbnet.h"
#include "lbmbedtls.h"

struct lbnet_context* lbnet_create_mbed_tls_context()
{
    struct lbnet_context* pnc = (struct lbnet_context*)lbmalloc(sizeof(struct lbnet_context));
    memset(pnc, 0, sizeof(struct lbnet_context));
    pnc->powner = lbnet_mbed_tls_create_conext();

    pnc->pnet_connect_imp           = lbnet_mbed_tls_connect;
    pnc->pnet_read_imp              = lbnet_mbed_tls_read_imp;
    pnc->pnet_write_imp             = lbnet_mbed_tls_write_imp;
    pnc->pnet_set_read_timeout_imp  = lbnet_mbed_tls_set_read_timeout;
    pnc->pnet_set_write_timeout_imp = lbnet_mbed_tls_set_write_timeout;
    pnc->pnet_disconnect_imp        = lbnet_mbed_tls_disconect;
    pnc->pnet_close_imp             = lbnet_mbed_tls_close;
    pnc->enet_type                  = e_net_type_mbed_tls;
    return pnc;
}

struct lbnet_context* lbnet_open(e_net_type net_type)
{
    struct lbnet_context* pnc = NULL;
    switch(net_type)
    {
        case e_net_type_tcp:
            return NULL;
        case e_net_type_ssl:
            return NULL;
        case e_net_type_mbed_tls:
            pnc = lbnet_create_mbed_tls_context();
            break;
        default:
        return NULL;
    }

    return pnc;
}

int lbnet_connect(struct lbnet_context* pnc, const char* host, int port)
{
    /*lbinfo("%s(pnc:%p, host:%s, port:%d)\n", __FUNCTION__, pnc, host, port);
    lbcheck_value(NULL == pnc || NULL == host || port <= 0, -1, "Invalid parameter, pnc:%p, host:%s, port:%d\n", pnc, host, port);
    lbcheck_pointer(pnc->pnet_connect_imp, -1, "function pnet_connect_imp:%p not implement\n", pnc->pnet_connect_imp);
    lbinfo("pnc->pnet_connect_imp:%p(pnc->powner:%p, host:%s, port:%d), lbnet_mbed_tls_connect:%p\n", pnc->pnet_connect_imp, pnc->powner, host, port, lbnet_mbed_tls_connect);
    */
    int ret = pnc->pnet_connect_imp(pnc->powner, host, port);
    lbinfo("ret:%d = pnc->pnet_connect_imp(pnc->powner, host, port)\n", ret);
    return ret;
}

int lbnet_read(struct lbnet_context* pnc, char* pdata, int len)
{
    int ret = -1;
    int offset = 0;
    long begin_time = lbget_sys_time();
    lbcheck_value(NULL == pnc || NULL == pdata || len <= 0, -1, "Invalid parameter, pnc:%p, pdata:%p, len:%d\n", pnc, pdata, len);
    lbcheck_pointer(pnc->pnet_read_imp, -1, "function pnet_read_imp:%p not implement\n", pnc->pnet_read_imp);
   
    do
    {
       ret = pnc->pnet_read_imp(pnc->powner, pdata + offset, len - offset);
       if(ret <= 0)
       {
           if(lbget_sys_time() - begin_time < pnc->lread_timeout_ms && (EAGAIN == errno || EINTR == errno || EWOULDBLOCK == errno))
           {
               usleep(50000);
               continue;
           }

           if(0 == errno)
           {
               ret = -1;
               lberror("pnc->pnet_read_imp faied, len:%d, ret:%d, spend time:%lu\n", len, ret, lbget_sys_time() - begin_time);
           }
           else
           {
               ret = errno > 0 ? (-errno) : errno;
               lberror("ret:%d = pnc->pnet_read_imp(pnc:%p, pdata + offset, len - offset:%d) failed, reason:%s", ret, pnc, len - offset, strerror(errno));
           }
           break;
       }
       else
       {
           offset += ret;
       }
       
    } while (offset <= 0);
    return offset;
}

int lbnet_write(struct lbnet_context* pnc, const char* pdata, int len)
{
    int ret = -1;
    int offset = 0;
    long begin_time = lbget_sys_time();
    lbcheck_value(NULL == pnc || NULL == pdata || len <= 0, -1, "Invalid parameter, pnc:%p, pdata:%p, len:%d\n", pnc, pdata, len);
    lbcheck_pointer(pnc->pnet_write_imp, -1, "function pnet_write_imp:%p not implement\n", pnc->pnet_write_imp);

    
    do
    {
       ret = pnc->pnet_write_imp(pnc->powner, pdata + offset, len - offset);
       if(ret <= 0)
       {
           if(lbget_sys_time() - begin_time < pnc->lwrite_timeout_ms && (EAGAIN == errno || EINTR == errno || EWOULDBLOCK == errno))
           {
               usleep(50000);
               continue;
           }

           if(0 == errno)
           {
               ret = -1;
               lberror("pnet_write_imp faied, len:%d, ret:%d, spend time:%lu\n", len, ret, lbget_sys_time() - begin_time);
           }
           else
           {
               ret = errno > 0 ? (-errno) : errno;
               lberror("ret:%d = pnet_write_imp(pnc:%p, pdata + offset, len - offset:%d) failed, reason:%s", ret, pnc, len - offset, strerror(errno));
           }
           break;
       }
       else
       {
           offset += ret;
       }
       
    } while (offset < len);
    return offset;
}

int lbnet_set_read_timeout(struct lbnet_context* pnc, int timeout_ms)
{
    lbcheck_pointer(pnc, -1, "Invalid parameter, pnc:%p\n", pnc);
    lbcheck_pointer(pnc->pnet_set_read_timeout_imp, -1, "function pnet_set_read_timeout_imp:%p not implement\n", pnc->pnet_set_read_timeout_imp);
    
    return pnc->pnet_set_read_timeout_imp(pnc->powner, timeout_ms);
}

int lbnet_set_write_timeout(struct lbnet_context* pnc, int timeout_ms)
{
    lbcheck_pointer(pnc, -1, "Invalid parameter, pnc:%p\n", pnc);
    lbcheck_pointer(pnc->pnet_set_read_timeout_imp, -1, "function pnet_set_read_timeout_imp:%p not implement\n", pnc->pnet_set_read_timeout_imp);
    
    return pnc->pnet_set_write_timeout_imp(pnc->powner, timeout_ms);
}

int lbnet_disconect(struct lbnet_context* pnc)
{
    lbcheck_pointer(pnc, -1, "Invalid parameter, pnc:%p\n", pnc);
    lbcheck_pointer(pnc->pnet_set_read_timeout_imp, -1, "function pnet_set_read_timeout_imp:%p not implement\n", pnc->pnet_set_read_timeout_imp);

    return pnc->pnet_disconnect_imp(pnc);
}

void lbnet_close(struct lbnet_context** ppnc)
{
    if(ppnc)
    {
        struct lbnet_context* pnc = *ppnc;
        lbinfo("%s pnc->powner:%p, pnc->pnet_close_imp:%p\n", __FUNCTION__, pnc->powner, pnc->pnet_close_imp);
        if(pnc->powner && pnc->pnet_close_imp)
        {
            pnc->pnet_close_imp(pnc->powner);
            lbinfo("%s pnc->pnet_close_imp(pnc->powner:%p)\n", __FUNCTION__, pnc->powner);
        }

        lbfree(pnc);
        *ppnc = pnc = NULL;
        lbinfo("%s end\n", __FUNCTION__);
    }
}