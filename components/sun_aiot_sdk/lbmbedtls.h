//#ifndef _LAZY_BEAR_MBED_TLS_H_
//#define _LAZY_BEAR_MBED_TLS_H_
#pragma once
#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include "sv_log.h"
//#include <srs_kernel_error.hpp>
//#include <srs_kernel_utility.hpp>
#ifdef WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#pragma warning( disable : 4996)
#else
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
// open ssl include header
//#include "lbnet.h"
//#include "lburl.h"

void* lbnet_mbed_tls_conenct_test(void* handle, const char* host, int port);

void* lbnet_mbed_tls_create_conext();

//int mbed_tls_connect(void* phandle, const char* host, int port);
int lbnet_mbed_tls_connect(void* phandle, const char* host, int port);

int lbnet_mbed_tls_read_imp(void* phandle, char* pdata, int len);

int lbnet_mbed_tls_write_imp(void* phandle, const char* pdata, int len);

//int lbnet_mbed_tls_read(void* phandle, char* pdata, int len);

//int lbnet_mbed_tls_write(void* phandle, const char* pdata, int len);

int lbnet_mbed_tls_set_read_timeout(void* phandle, int timeout_ms);

int lbnet_mbed_tls_set_write_timeout(void* phandle, int timeout_ms);

int lbnet_mbed_tls_disconect(void* phandle);

void lbnet_mbed_tls_close(void* phandle);

//#endif