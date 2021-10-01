/*
 * list.h
 *
 * Copyright (c) 2019 sunvalley
 * Copyright (c) 2019 dawson <dawson.wu@sunvalley.com.cn>
 */

#pragma once
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include "lbmacro.h"

#ifndef lbmutex
#if defined(WIN32) || defined(WIN64)
#define lbmutex                     CRITICAL_SECTION
#define lbmutex_init(px)            InitializeCriticalSection(px)
#define lbmutex_deinit(px)         DeleteCriticalSection(px)
#define lbmutex_lock(px)            EnterCriticalSection(px)
#define lbmutex_unlock(px)          LeaveCriticalSection(px)
#else
#define lbmutex                     pthread_mutex_t
#define lbmutex_recursive_init(px)      pthread_mutexattr_t attr; \
                                        pthread_mutexattr_init(&attr); \
                                        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); \
                                        pthread_mutex_init(px, &attr); \
                                        pthread_mutexattr_destroy(&attr);

#define lbmutex_init(px)            pthread_mutex_init(px, NULL)
#define lbmutex_deinit(px)          pthread_mutex_destroy(px)
#define lbmutex_lock(px)            pthread_mutex_lock(px)
#define lbmutex_unlock(px)          pthread_mutex_unlock(px)
#endif
#endif

struct lblist_node
{
    struct lblist_node* prev;
    struct lblist_node* pnext;
    void* pitem;
};

// list_context
struct lblist_ctx
{
    struct lblist_node* head;
    struct lblist_node* tail;
    int count;
    int max_num;
    lbmutex* pmutex;
};

static struct lblist_ctx* lblist_context_create(int max_num)
{
    struct lblist_ctx* plist = (struct lblist_ctx*)malloc(sizeof(struct lblist_ctx));
    memset(plist, 0, sizeof(struct lblist_ctx));
    plist->pmutex = (lbmutex*)malloc(sizeof(lbmutex));
    lbmutex_recursive_init(plist->pmutex);
    plist->max_num = max_num;

    return plist;
}

static void lblist_context_close(struct lblist_ctx** pplistctx)
{
    if(pplistctx && *pplistctx)
    {
        struct lblist_ctx* plist = *pplistctx;
        lbmutex_deinit(plist->pmutex);
        free(plist->pmutex);
        free(plist);
        *pplistctx = NULL;
    }
}

static int lblist_push(struct lblist_ctx*  plist, void* pitem)
{
    if(NULL == plist)
    {
        assert(0);
        return -1;
    }
    lbmutex_lock(plist->pmutex);
    //printf("lblist_push before malloc\n");
    struct lblist_node* pnode = (struct lblist_node*)malloc(sizeof(struct lblist_node));
    //printf("lblist_push pnode:%p = malloc\n", pnode);
    pnode->pitem = pitem;
    if(plist->tail)
    {
        plist->tail->pnext = pnode;
        pnode->prev = plist->tail;
        pnode->pnext = NULL;
        plist->tail = pnode;
        plist->count++;
    }
    else
    {
        plist->head = pnode;
        plist->tail = pnode;
        pnode->prev = NULL;
        pnode->pnext = NULL;
        plist->count++;
    }
    assert(plist->head);
    lbmutex_unlock(plist->pmutex);
    return 0;
}

static void* lblist_pop(struct lblist_ctx*  plist)
{
    lbmutex_lock(plist->pmutex);
    if(NULL == plist && plist->count <= 0)
    {
        lbmutex_unlock(plist->pmutex);
        return NULL;
    }
    
    assert(plist->head);
    struct lblist_node* pnode = plist->head;
    plist->head = pnode->pnext;
    plist->count--;
    void* pitem = pnode->pitem;
    free(pnode);
    if(NULL == plist->head)
    {
        plist->tail = NULL;
        assert(0 == plist->count);
    }
    //assert(plist->head);
    lbmutex_unlock(plist->pmutex);
    return pitem;
}

static void* lblist_front(struct lblist_ctx* plist)
{
    lbmutex_lock(plist->pmutex);
    if(NULL == plist && plist->count <= 0)
    {
        lbmutex_unlock(plist->pmutex);
        return NULL;
    }
    assert(plist->head);
    void* pitem = plist->head->pitem;
    lbmutex_unlock(plist->pmutex);
    return pitem;
}

static int lblist_remove(struct lblist_ctx* plist, struct lblist_node* pnode)
{
    lbmutex_lock(plist->pmutex);
    if(NULL == plist && plist->count <= 0)
    {
        lbmutex_unlock(plist->pmutex);
        return -1;
    }

    if(1 == plist->count)
    {
        plist->head = NULL;
        plist->tail = NULL;
    }
    else
    {
        // pnode is the first node
        if(pnode->prev)
        {
            pnode->prev->pnext = pnode->pnext;
        }
        else
        {
            plist->head = pnode->pnext;
        }

        // pnode is the last node
        if(pnode->pnext)
        {
            pnode->pnext->prev = pnode->prev;
        }
        else
        {
            plist->tail = pnode->prev;
        }
    }
    
    plist->count--;
    free(pnode);
    lbmutex_unlock(plist->pmutex);
    return 0;
}

static int lblist_size(struct lblist_ctx* plist)
{
    return plist ? plist->count : 0;
}

#define BEGIN_ENUM_LIST(plist, ptmp) {struct lblist_node* ptmp = NULL; for(ptmp = plist ? plist->head : NULL; ptmp != NULL; ptmp = ptmp->pnext){
#define END_ENUM_LIST }}

