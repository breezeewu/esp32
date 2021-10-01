#ifndef _LAZY_BEAR_URL_H_
#define _LAZY_BEAR_URL_H_
#include "lbmacro.h"
#include <stdbool.h>
#define MAX_URL_SIZE    1024

typedef struct{
    char*   purl;
    char*   pschema;
    char*   phost;
    int     port;
    char*   ppath;
    char*   pparam;
    bool    bis_ssl;
    } lburl_context;


lburl_context*  lburl_context_open(const char* url);

bool lburl_is_https(lburl_context* puc);

void lburl_context_close(lburl_context** ppuc);

int lburl_parser_local_path(const char* path, char** dir, char** name, char** suffix);

#endif