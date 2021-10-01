/*
 * lbthread.h
 *
 * Copyright (c) 2021 lazybrear
 * Copyright (c) 2021 dawson <dawson.wu@sunvalley.com.cn>
 */
#pragma once
#include <stdbool.h>
#include <pthread.h>

typedef void* (*lbthread_proc)(void* arg);



struct lbthread_context* lbthread_open(const char* pname, void* owner, lbthread_proc thead_func);

int lbthread_set_detach(struct lbthread_context* ptc, bool detach);

int lbthread_start(struct lbthread_context* ptc);

int lbthread_stop(struct lbthread_context* ptc);

bool lbthread_is_alive(struct lbthread_context* ptc);

void* lbthread_get_owner(struct lbthread_context* ptc);

void lbthread_close(struct lbthread_context** pptc);

/*#ifndef lbmutex
#define lbmutex          struct lbmutex_ctx

lbmutex* lbmutex_open(bool request);

int lbmutex_lock(lbmutex* pmutex);

int lbmutex_unlock(lbmutex* pmutex);

void lbmutex_close(lbmutex** ppmutex);
#endif*/