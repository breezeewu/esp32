/*
 * list.h
 *
 * Copyright (c) 2019 sunvalley
 * Copyright (c) 2019 dawson <dawson.wu@sunvalley.com.cn>
 */

#ifndef _CACHE_BUFFER_H_
#define _CACHE_BUFFER_H_
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
struct lbcache_buffer_context
{
    uint8_t*    pcache_buf;
    int         nmax_cache_len;
    int         nbegin;
    int         nend;
    pthread_mutex_t* pmutex;
};

#ifndef lbtrace
#define lbtrace printf
#endif
#ifndef lberror
#define lberror printf
#endif

#define INIT_RECURSIVE_MUTEX(pmutex) pthread_mutexattr_t attr; \
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); \
pthread_mutex_init(pmutex, &attr);

struct lbcache_buffer_context* lbcache_buffer_open(int max_cache_size)
{
    struct lbcache_buffer_context* pcache = (struct lbcache_buffer_context*)malloc(sizeof(struct lbcache_buffer_context));
    pcache->pcache_buf = (uint8_t*)malloc(max_cache_size);
    pcache->nmax_cache_len = max_cache_size;
    pcache->nbegin = 0;
    pcache->nend = 0;
    /*pcache->pmutex = malloc(sizeof(pthread_mutex_t));
    assert(pcache->pmutex);
    INIT_RECURSIVE_MUTEX(pcache->pmutex);*/

    return pcache;
}

static void lbcache_buffer_closep(struct lbcache_buffer_context** ppcache_ctx)
{
    if(ppcache_ctx && *ppcache_ctx)
    {
        struct lbcache_buffer_context* pcache = *ppcache_ctx;
        //pthread_mutex_lock(pcache->pmutex);
        free(pcache->pcache_buf);
        //pthread_mutex_unlock(pcache->pmutex);
        //pthread_mutex_destroy(pcache->pmutex);
        //free(pcache->pmutex);
        free(pcache);
        *ppcache_ctx = NULL;
    }
}

void lbcache_buffer_align(struct lbcache_buffer_context* pcache)
{
    if(NULL == pcache || NULL == pcache->pcache_buf)
    {
        lberror("Invalid parameter, pcache:%p, pcache->pcache_buf:%p\n", pcache, pcache->pcache_buf);
        return ;
    }

    //int len = pcache->nend - pcache->nbegin;
    memmove(pcache->pcache_buf, pcache->pcache_buf + pcache->nbegin, pcache->nend - pcache->nbegin);
    pcache->nend = pcache->nend - pcache->nbegin;
    pcache->nbegin = 0;
    return ;
    //pthread_mutex_lock(pcache->pmutex);
    /*if(pcache->nbegin > 0)
    {
        int bmalloc = 0;
        uint8_t* pcopybuf = NULL;
        int copylen = pcache->nend - pcache->nbegin;
        if(pcache->nbegin < pcache->nend - pcache->nbegin)
        {
            pcopybuf = malloc(copylen);
            memcpy(pcopybuf, pcache->pcache_buf + pcache->nbegin, copylen);
            bmalloc = 1;
        }
        else
        {
            pcopybuf = pcache->pcache_buf + pcache->nbegin;
        }
        memcpy(pcache->pcache_buf, pcopybuf, copylen);
        pcache->nbegin = 0;
        pcache->nend = copylen;
        if(bmalloc)
        {
            free(pcopybuf);
        }
    }
    //pthread_mutex_unlock(pcache->pmutex);*/
}

int lbcache_buffer_deliver_data(struct lbcache_buffer_context* pcache, uint8_t* pdata, int len)
{
    if(!pcache || !pdata)
    {
        lberror("Invalid parameter !pcache:%p || !pdata:%p\n", pcache, pdata);
        return -1;
    }
    //pthread_mutex_lock(pcache->pmutex);
    if(pcache->nmax_cache_len - pcache->nend + pcache->nbegin < len)
    {
        lberror("no enought cache buffer for data cache, remain:%d, len:%d\n", pcache->nmax_cache_len - pcache->nend + pcache->nbegin, len);
        //pthread_mutex_unlock(pcache->pmutex);
        return -1;
    }
    
    if(pcache->nmax_cache_len - pcache->nend < len)
    {
        lbcache_buffer_align(pcache);
    }
    memcpy(pcache->pcache_buf + pcache->nend, pdata, len);
    pcache->nend += len;
    //pthread_mutex_unlock(pcache->pmutex);
    return 0;
}

int lbcache_buffer_get_remain(struct lbcache_buffer_context* pcache)
{
    if(NULL == pcache)
    {
        lberror("Invalid parameter, pcache:%p\n", pcache);
        return -1;
    }

    return (pcache->nend - pcache->nbegin)%pcache->nmax_cache_len;
}

int lbcache_buffer_get_free(struct lbcache_buffer_context* pcache)
{
    if(NULL == pcache)
    {
        lberror("Invalid parameter, pcache:%p\n", pcache);
        return -1;
    }

    return pcache->nmax_cache_len - lbcache_buffer_get_remain(pcache);
}

int lbcache_buffer_fetch_data(struct lbcache_buffer_context* pcache, uint8_t* pdata, int len)
{
    if(!pcache || !pdata)
    {
        lberror("Invalid parameter !pcache:%p || !pdata:%p\n", pcache, pdata);
        return -1;
    }
    
    //pthread_mutex_lock(pcache->pmutex);
    if(pcache->nend - pcache->nbegin < len)
    {
        //pthread_mutex_unlock(pcache->pmutex);
        return 0;
    }
    
    memcpy(pdata, pcache->pcache_buf + pcache->nbegin, len);
    pcache->nbegin += len;
    //pthread_mutex_unlock(pcache->pmutex);
    return len;
}

int lbcache_buffer_try_fetch(struct lbcache_buffer_context* pcache, uint8_t** ppdata)
{
    if(NULL == pcache || NULL == ppdata)
    {
        lberror("%s(pcache:%p, ppdata:%p)\n", __FUNCTION__, pcache, ppdata);
        return -1;
    }

    *ppdata = pcache->pcache_buf + pcache->nbegin;
    return lbcache_buffer_get_remain(pcache); 
}

int lbcache_buffer_skip(struct lbcache_buffer_context* pcache, int offset)
{
    if(pcache && bcache_buffer_get_remain(pcache) >= offset)
    {
        pcache->nbegin += offset;
        if(pcache->nbegin == pcache->nend)
        {
            pcache->nbegin = pcache->nend = 0;
        }

        return 0;
    }

    return -1;
}