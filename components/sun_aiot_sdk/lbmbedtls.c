//#include "lbnet.h"
#include "lbmbedtls.h"
#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "esp_crt_bundle.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lbmbedtls.h"
#include "lbmacro.h"
//#include "esp_crt_bundle.h"

#define MIN_SOCKET_TIMEOUT_IN_MS			5000
#define MAX_SOCKET_TIMEOUT_IN_MS			120000
int g_ncount = 0;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET  ~0
#endif
typedef struct{
    mbedtls_entropy_context*        entropy;
    mbedtls_ctr_drbg_context*       ctr_drbg;
    mbedtls_ssl_context*            ssl;
    mbedtls_x509_crt*               cacert;
    mbedtls_ssl_config*             conf;
    mbedtls_net_context*            fd;

} lbnet_mbed_tls_context;
const char* preq = "POST /oauth/refresh-token? HTTP/1.1\r\nHost: iot-api-sit.sunvalleycloud.com\r\n\
Connection: Keep-Alive\r\n\
User-Agent: sunvalley device SDK V2021.06.25\r\n\
Content-Length: 197\r\n\
Content-Type: application/json;charset=UTF-8\r\n\r\n\
{\"client_id\":\"95f225e106c0447787a0882c61345cf4\",\"client_secret\":\"a6bb6211db7c116afa8623e9e97e3a1e\",\"scope\":\"all\",\"grant_type\":\"refresh_token\",\"refresh_token\":\"338177f3-8f08-43c4-9ed7-121cd4f113b4\"}";

void* lbnet_mbed_tls_conenct_test(void* handle, const char* host, int port)
{
    lbinfo("%s(handle:%p, host:%s, port:%d) begin111\n", __FUNCTION__, handle, host, port);
    if(NULL == handle)
    {
        handle = lbnet_mbed_tls_create_conext();
        lbinfo("%s handle:%p = lbnet_mbed_tls_create_conext()\n", __FUNCTION__, handle);
    }
    
    int ret = lbnet_mbed_tls_connect(handle, host, port);
    lbinfo("%s ret:%d = lbnet_mbed_tls_connect\n", __FUNCTION__, ret);
    if(0 != ret)
    {
        lbnet_mbed_tls_close(handle);
        handle = NULL;
    }
    ret = lbnet_mbed_tls_write_imp(handle, preq, strlen(preq));
    lbinfo("ret:%d = lbnet_mbed_tls_write_imp(handle, preq:%s, strlen(preq))\n", ret, preq);
    char* pbuf = (char*)malloc(1024);
    memset(pbuf, 0, 1024);
    ret = lbnet_mbed_tls_read_imp(handle, pbuf, 1024);
    lbinfo("ret:%d = lbnet_mbed_tls_read_imp(handle, pbuf:%s, 1024)\n", ret, pbuf);
    lbnet_mbed_tls_close(handle);
    handle = NULL;
    lbfree(pbuf);
    lbinfo("%s end\n", __FUNCTION__);
    return handle;
}
//lbnet_context* lbnet_close_context(lbnet_context* pnc);
void* lbnet_mbed_tls_create_conext()
{
    lbinfo("%s begin\n", __FUNCTION__);
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_create_default();
    lbnet_mbed_tls_context* pmtc = (lbnet_mbed_tls_context*)lbmalloc(sizeof(lbnet_mbed_tls_context));
    //lbinfo("pmtc:%p = (lbnet_mbed_tls_context*)lbmalloc(sizeof(lbnet_mbed_tls_context):%d)\n", pmtc, sizeof(lbnet_mbed_tls_context));
    memset(pmtc, 0, sizeof(lbnet_mbed_tls_context));
    //lbinfo("before mbedtls_ssl_init(pmtc->ssl:%p), sizeof(mbedtls_ssl_context):%ld\n", pmtc->ssl, sizeof(mbedtls_ssl_context));
    pmtc->entropy = (mbedtls_entropy_context*)lbmalloc(sizeof(mbedtls_entropy_context));
    memset(pmtc->entropy, 0, sizeof(mbedtls_entropy_context));

    pmtc->ctr_drbg = (mbedtls_ctr_drbg_context*)lbmalloc(sizeof(mbedtls_ctr_drbg_context));
    memset(pmtc->entropy, 0, sizeof(mbedtls_ctr_drbg_context));

    pmtc->ssl = (mbedtls_ssl_context*)lbmalloc(sizeof(mbedtls_ssl_context));
    memset(pmtc->ssl, 0, sizeof(mbedtls_ssl_context));

    pmtc->cacert = (mbedtls_x509_crt*)lbmalloc(sizeof(mbedtls_x509_crt));
    memset(pmtc->cacert, 0, sizeof(mbedtls_x509_crt));

    pmtc->conf = (mbedtls_ssl_config*)lbmalloc(sizeof(mbedtls_ssl_config));
    memset(pmtc->conf, 0, sizeof(mbedtls_ssl_config));

    pmtc->fd = (mbedtls_net_context*)lbmalloc(sizeof(mbedtls_net_context));
    memset(pmtc->fd, 0, sizeof(mbedtls_net_context));

    //lbinfo("memset(pmtc->ssl, 0, sizeof(mbedtls_ssl_context):%d)\n", sizeof(mbedtls_ssl_context));
    //mbedtls_ssl_init(pmtc->ssl);
    lbinfo("after mbedtls_ssl_init(pmtc->ssl)\n");
    return pmtc;
}

int lbnet_mbed_tls_connect(void* phandle, const char* host, int port)
{
    //lbinfo("%s() begin 1\n", __FUNCTION__);
    int ret = -1;
    char str_port[20];
    unsigned long begin_time = lbget_sys_time();
    lbnet_mbed_tls_context* pmtc = (lbnet_mbed_tls_context*)phandle;
    //lbinfo("lbnet_mbed_tls_connect begin 1\n");
    
    if(NULL == pmtc || NULL == host || port <= 0)
    {
        lberror("Invalid parameter, pmtc:%p, host:%s, port:%d\n", pmtc, host, port);
        return -1;
    }
    lbinfo("%s(phandle:%p, host:%s, port:%d)2\n", __FUNCTION__, phandle, host, port);
    // init mbed tls
    mbedtls_ssl_init(pmtc->ssl);
    //lbinfo("mbedtls_ssl_init3\n");
    
    mbedtls_x509_crt_init(pmtc->cacert);
    //lbinfo("mbedtls_x509_crt_init\n");
    mbedtls_ctr_drbg_init(pmtc->ctr_drbg);
    //lbinfo("mbedtls_ctr_drbg_init\n");
   // ESP_LOGI(TAG, "Seeding the random number generator");

    mbedtls_ssl_config_init(pmtc->conf);
    //lbinfo("mbedtls_ssl_config_init\n");
    mbedtls_entropy_init(pmtc->entropy);
    //lbinfo("mbedtls_ctr_drbg_seed\n");
    ret = mbedtls_ctr_drbg_seed(pmtc->ctr_drbg, mbedtls_entropy_func, pmtc->entropy, NULL, 0);
    lbcheck_return(ret, "ret:%d = mbedtls_ctr_drbg_seed failed\n", ret);
    //lbinfo("ret:%d = mbedtls_ctr_drbg_seed\n", ret);
    ret = esp_crt_bundle_attach(pmtc->conf);
    if(ret < 0)
    {
        lberror("ret:%d = esp_crt_bundle_attach(&conf)", ret);
        return ret;
    }
    //lbinfo("ret:%d = esp_crt_bundle_attach(&conf)", ret);

     // Hostname set here should match CN in server certificate
    ret = mbedtls_ssl_set_hostname(pmtc->ssl, host);
    lbcheck_return(ret, "ret:%d = mbedtls_ssl_set_hostname(pmtc->ssl, host:%s)\n", ret, host);
    //lbinfo("ret:%d = mbedtls_ssl_set_hostname(pmtc->ssl, host:%s)\n", ret, host);
    ret = mbedtls_ssl_config_defaults(pmtc->conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT);
    lbcheck_return(ret, "ret:%d = mbedtls_ssl_config_defaults\n", ret);
    //lbinfo("ret:%d = mbedtls_ssl_set_hostname(pmtc->ssl, host:%s)\n", ret, host);
    ret = mbedtls_ssl_config_defaults(pmtc->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    lbcheck_return(ret, "ret:%d = mbedtls_ssl_config_defaults\n", ret);
    //lbinfo("ret:%d = mbedtls_ssl_config_defaults\n", ret);
    /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
       a warning if CA verification fails but it will continue to connect.

       You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
    */
    mbedtls_ssl_conf_authmode(pmtc->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    //lbinfo("mbedtls_ssl_conf_authmode\n");
    mbedtls_ssl_conf_ca_chain(pmtc->conf, pmtc->cacert, NULL);
    //lbinfo("mbedtls_ssl_conf_ca_chain\n");
    mbedtls_ssl_conf_rng(pmtc->conf, mbedtls_ctr_drbg_random, pmtc->ctr_drbg);
    //lbinfo("mbedtls_ssl_conf_rng\n");
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(pmtc->conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
#endif
    ret = mbedtls_ssl_setup(pmtc->ssl, pmtc->conf);
    lbcheck_return(ret, "ret:%d = mbedtls_ssl_setup(&ssl, &conf)\n", ret);
    //lbinfo("%s ret:%d = mbedtls_ssl_setup(pmtc->ssl, pmtc->conf), spend time:%lu\n", __FUNCTION__, ret, lbget_sys_time() - begin_time);
    mbedtls_net_init(pmtc->fd);
    sprintf(str_port, "%d", port);
    //lbinfo("mbedtls_net_init(pmtc->server_fd), str_port:%s\n", str_port);
    ret = mbedtls_net_connect(pmtc->fd, host, str_port, MBEDTLS_NET_PROTO_TCP);
    //lbcheck_return(ret, "ret:%d = mbedtls_net_connect(pmtc->server_fd, host:%s, port:%d, MBEDTLS_NET_PROTO_TCP\n", ret, host, port);
    lbinfo("ret:%d = mbedtls_net_connect(pmtc->server_fd, host:%s, port:%d, MBEDTLS_NET_PROTO_TCP)\n", ret, host, port);
    lbinfo("%s ret:%d = mbedtls_net_connect, spend time:%lu\n", __FUNCTION__, ret, lbget_sys_time() - begin_time);
    mbedtls_ssl_set_bio(pmtc->ssl, pmtc->fd, mbedtls_net_send, mbedtls_net_recv, NULL);
    //lbcheck_return(ret, "ret:%d = mbedtls_ssl_set_bio", ret);
    lbinfo("ret:%d = mbedtls_ssl_set_bio, spend time:%lu\n", ret, lbget_sys_time() - begin_time);
    ret = mbedtls_ssl_handshake(pmtc->ssl);
    lbcheck_return(ret, "ret:%d = mbedtls_ssl", ret);
    lbinfo("ret:%d = mbedtls_ssl_handshake, spend time:%lu\n", ret, lbget_sys_time() - begin_time);
    ret = mbedtls_ssl_get_verify_result(pmtc->ssl);
    //lbinfo("ret:%d = mbedtls_ssl_get_verify_result(pmtc->ssl)\n", ret);
    if(0 != ret)
    {
        char buf[512];
        memset(buf, 0, sizeof(buf));
        mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", ret);
        lberror("verification info: %s", buf);
    }
    else
    {
        lbtrace("Certificate verified.\n");
    }
    
    lbcheck_return(ret, "ret:%d = mbedtls_ssl_get_verify_result", ret);
    lbinfo("%s end, ret:%d, connect spend time:%lu\n", __FUNCTION__, ret, lbget_sys_time() - begin_time);
    return ret;
}

int lbnet_mbed_tls_read_imp(void* phandle, char* pdata, int len)
{
    int ret = 0;
    lbnet_mbed_tls_context* pmtc = (lbnet_mbed_tls_context*)phandle;
    lbcheck_value(NULL == pmtc || NULL == pdata || len <= 0, -1, "Invalid parameter, pmtc:%p, pdata:%p, len:%d\n", pmtc, pdata, len);

    ret = mbedtls_ssl_read(pmtc->ssl, (unsigned char*)pdata, len);
    //lbinfo("ret:%d = %s(pdata:%s, len:%d)\n", ret, __FUNCTION__, pdata, len);
    return ret;
}

int lbnet_mbed_tls_write_imp(void* phandle, const char* pdata, int len)
{
    lbnet_mbed_tls_context* pmtc = (lbnet_mbed_tls_context*)phandle;
    lbcheck_value(NULL == pmtc || NULL == pdata || len <= 0, -1, "Invalid parameter, pmtc:%p, pdata:%p, len:%d\n", pmtc, pdata, len);
    //lbinfo("%s(pdata:%s, len:%d)\n", __FUNCTION__, pdata, len);
    return mbedtls_ssl_write(pmtc->ssl, (const unsigned char*)pdata, len);
}


int lbnet_mbed_tls_set_read_timeout(void* phandle, int timeout_ms)
{
    //lbnet_mbed_tls_context* pmtc = (lbnet_mbed_tls_context*)phandle;
    //lbcheck_pointer(pmtc, -1, "Invalid parameter, pmtc:%p\n", pmtc);

    return -1;
}

int lbnet_mbed_tls_set_write_timeout(void* phandle, int timeout_ms)
{
    return -1;
}

int lbnet_mbed_tls_disconect(void* phandle)
{
    lbnet_mbed_tls_context* pmtc = (lbnet_mbed_tls_context*)phandle;
    lbcheck_pointer(pmtc, -1, "Invalid parameter, pmtc:%p\n", pmtc);
    mbedtls_net_free(pmtc->fd);
    mbedtls_x509_crt_free(pmtc->cacert);
    //mbedtls_ssl_close_notify(pmtc->ssl);
    mbedtls_ssl_free(pmtc->ssl);
    mbedtls_ssl_config_free(pmtc->conf);
    mbedtls_ctr_drbg_free(pmtc->ctr_drbg);
    mbedtls_entropy_free(pmtc->entropy);

    return 0;
}

void lbnet_mbed_tls_close(void* phandle)
{
    lbnet_mbed_tls_context* pmtc = (lbnet_mbed_tls_context*)phandle;
    if(pmtc)
    {
        //lbinfo("%s(pmtc:%p)\n", __FUNCTION__, pmtc);
        lbnet_mbed_tls_disconect(pmtc);
        lbfree(pmtc->entropy);
        lbfree(pmtc->ctr_drbg);
        lbfree(pmtc->ssl);
        lbfree(pmtc->cacert);
        lbfree(pmtc->conf);
        lbfree(pmtc->fd);

        lbfree(phandle);
        //lbinfo("%s end\n", __FUNCTION__);
    }
}
