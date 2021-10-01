#ifndef _LAZY_BEAR_MBED_TLS_H_
#define _LAZY_BEAR_MBED_TLS_H_
#include <stdio.h>
#include <string.h>
#include <assert.h>


typedef enum{
    e_net_type_unknown  = -1,
    e_net_type_tcp      = 0,
    e_net_type_ssl      = 1,
    e_net_type_mbed_tls = 2
} e_net_type;

typedef int (*lbnet_connect_imp)(void* phandle, const char* host, int port);
typedef int (*lbnet_read_imp)(void* phandle, char* pdata, int len);
typedef int (*lbnet_write_imp)(void* phandle, const char* pdata, int len);
typedef int (*lbnet_set_read_timeout_imp)(void* phandle, int timeout_ms);
typedef int (*lbnet_set_write_timeout_imp)(void* phandle, int timeout_ms);
typedef int (*lbnet_disconnect_imp)(void* phandle);
typedef void (*lbnet_close_imp)(void* phandle);

struct lbnet_context{
    void*                           powner;
    lbnet_connect_imp               pnet_connect_imp;
    lbnet_read_imp                  pnet_read_imp;
    lbnet_write_imp                 pnet_write_imp;
    lbnet_set_read_timeout_imp      pnet_set_read_timeout_imp;
    lbnet_set_write_timeout_imp     pnet_set_write_timeout_imp;
    lbnet_disconnect_imp            pnet_disconnect_imp;
    lbnet_close_imp                 pnet_close_imp;

    long    lread_timeout_ms;
    long    lwrite_timeout_ms;

    long    ltotal_read_bytes;
    long    ltotal_write_bytes;

    e_net_type  enet_type;
};

struct lbnet_context* lbnet_open(e_net_type net_type);

int lbnet_connect(struct lbnet_context* pnc, const char* host, int port);

int lbnet_read(struct lbnet_context* pnc, char* pdata, int len);

int lbnet_write(struct lbnet_context* pnc, const char* pdata, int len);

int lbnet_read(struct lbnet_context* pnc, char* pdata, int len);

int lbnet_set_read_timeout(struct lbnet_context* pnc, int timeout_ms);

int lbnet_set_write_timeout(struct lbnet_context* pnc, int timeout_ms);

int lbnet_disconect(struct lbnet_context* pnc);

void lbnet_close(struct lbnet_context** ppnc);

/*typedef struct{
    int             fd;
    SSL_CTX*		pssl_ctx;
	SSL*			pssl;
    char*           pkey_pwd;
    long            ltotal_read_bytes;
    long            ltotal_write_bytes;
    long            lread_timeout;
    long            lwrite_timeout;
} lbtcp_context;


lbtcp_context* lbtcp_open(const char* phost, int port, bool is_ssl);

int lbtcp_read(lbtcp_context* psc, char* pdata, int len);

int lbtcp_write(lbtcp_context* psc, const char* pdata, int len);

int lbtcp_set_read_timeout(lbtcp_context* psc, int timeout_ms);

int lbtcp_set_write_timeout(lbtcp_context* psc, int timeout_ms);

void lbtcp_close(lbtcp_context** ppsc);
*/
#endif