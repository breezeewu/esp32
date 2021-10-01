/*
 * list.h
 *
 * Copyright (c) 2019 sunvalley
 * Copyright (c) 2019 dawson <dawson.wu@sunvalley.com.cn>
 */

#pragma once
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>

/*#ifndef lbtrace
#define lbtrace printf
#endif
#ifndef lberror
#define lberror printf
#endif

#define INIT_RECURSIVE_MUTEX(pmutex) pthread_mutexattr_t attr; \
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); \
pthread_mutex_init(pmutex, &attr);*/

struct lbcache_buffer_context* lbcache_buffer_open(int max_cache_size);

void lbcache_buffer_closep(struct lbcache_buffer_context** ppcache_ctx);

void lbcache_buffer_align(struct lbcache_buffer_context* pcache);

int lbcache_buffer_deliver_data(struct lbcache_buffer_context* pcache, uint8_t* pdata, int len);

int lbcache_buffer_get_remain(struct lbcache_buffer_context* pcache);

int lbcache_buffer_get_free(struct lbcache_buffer_context* pcache);

int lbcache_buffer_fetch_data(struct lbcache_buffer_context* pcache, uint8_t* pdata, int len);

int lbcache_buffer_try_fetch(struct lbcache_buffer_context* pcache, uint8_t** ppdata);

int lbcache_buffer_skip(struct lbcache_buffer_context* pcache, int offset);
