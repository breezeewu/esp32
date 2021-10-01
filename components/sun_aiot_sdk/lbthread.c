
#include "lbthread.h"
#include "lbmacro.h"

struct lbthread_context{
    bool        brun;
    bool        bloop;
    bool        bdetach;
    bool        bsystem;
    char*       thread_name;
    pthread_t   tid;
    void*       powner;
    lbthread_proc pthread_fun;
};

struct lbthread_context* lbthread_open(const char* pname, void* owner, lbthread_proc thead_func)
{
    struct lbthread_context* ptc = (struct lbthread_context*)lbmalloc(sizeof(struct lbthread_context));
    lbcheck_pointer(ptc, NULL, "lbmalloc out of memory, ptc:%p\n", ptc);
    memset(ptc, 0, sizeof(struct lbthread_context));
    ptc->powner = owner;
    ptc->pthread_fun = thead_func;
    if(pname)
    {
        lbstrcp(ptc->thread_name, pname);
    }
    return ptc;
}

int lbthread_set_detach(struct lbthread_context* ptc, bool detach)
{
    lbcheck_pointer(ptc, -1, "Invalid parameter, ptc:%p\n", ptc);
    ptc->bdetach = detach;
    return 0;
}

void* initial_thread_proc(void* arg)
{
    void* res = NULL;
    struct lbthread_context* ptc = (struct lbthread_context*)arg;
    lbcheck_pointer(ptc, NULL, "lbmalloc out of memory, ptc:%p\n", ptc);
    if(ptc->pthread_fun)
    {
        lbinfo("%s thread enter proc\n", ptc->thread_name);
        ptc->brun = true;
        ptc->bloop = true;
        res = ptc->pthread_fun(ptc);
        ptc->brun = false;

        if(!ptc->bdetach)
        {
            pthread_detach(ptc->tid);
        }
    }
    lbinfo("%s thread exit, res:%p\n", ptc->thread_name, res);
    return res;
}

int lbthread_start(struct lbthread_context* ptc)
{
    pthread_attr_t tattr;
    int res = 0;
    lbcheck_pointer(ptc, -1, "Invalid parameter, ptc:%p\n", ptc);
    do{
        res = pthread_attr_init(&tattr);
        lbcheck_return(res, "res:%d = pthread_attr_init(&tattr) failed\n", res);
        res = pthread_attr_setstacksize(&tattr, 64*1024);
        lbcheck_break(res, "res:%d = pthread_attr_setstacksize(&tattr, 64*1024) failed\n", res);
        /*if(ptc->bsystem)
        {
            res = pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
        }*/

        if(ptc->bdetach)
        {
            pthread_attr_setdetachstate(&tattr, ptc->bdetach);
        }

        res = pthread_create(&ptc->tid, &tattr, initial_thread_proc, ptc);
        lbcheck_break(res, "res:%d = pthread_create(&ptc->tid, &tattr, ptc->pthread_fun:%p, ptc->powner:%p) failed\n", res, ptc->pthread_fun, ptc->powner);
    }while(0);
    pthread_attr_destroy(&tattr);
    lbinfo("res = lbthread_start(ptc:%p)\n", res, ptc);
    return res;
}

int lbthread_stop(struct lbthread_context* ptc)
{
    long begin_time = lbget_sys_time();
    lbcheck_pointer(ptc, -1, "Invalid parameter, ptc:%p\n", ptc);
   
    if(ptc->brun)
    {
        ptc->bloop = false;
        //lbtrace("%s threads stop ptc->bdetach:%d\n", ptc->thread_name, ptc->bdetach);
        if(!ptc->bdetach)
        {
            lbtrace("%s pthread_join(ptc->tid, NULL)\n", ptc->thread_name);
            pthread_join(ptc->tid, NULL);
        }
        else
        {
            int count = 0;
            lbtrace("%s threads stop loop\n", ptc->thread_name);
            do
            {
                usleep(20000);
            } while (count++ < 100 && ptc->brun);
        }
        if(ptc->brun)
        {
            lbtrace("%s thread stop timeout, ptc->brun:%d, ptc->bloop:%d, spend time:%lu", ptc->thread_name, ptc->brun, ptc->bloop, lbget_sys_time() - begin_time);
        }
        lbinfo("%s thread stop end\n", ptc->thread_name);
    }

    memset(&ptc->tid, 0, sizeof(ptc->tid));
    return 0;
}

bool lbthread_is_alive(struct lbthread_context* ptc)
{
    return ptc ? ptc->bloop : false;
}


void* lbthread_get_owner(struct lbthread_context* ptc)
{
    return ptc ? ptc->powner : NULL;
}
void lbthread_close(struct lbthread_context** pptc)
{
    //lbinfo("lbthread_close pptc:%p\n", pptc);
    if(pptc && *pptc)
    {
        struct lbthread_context* ptc = *pptc;

        lbthread_stop(ptc);
        lbfree(ptc->thread_name);
        lbfree(ptc);
        *pptc = ptc = NULL;
    }
    //lbinfo("lbthread_close end\n");
}

/*typedef lbmutex_ctx{
    pthread_mutex_t
} lbmutex;

lbmutex* lbmutex_open(bool request);

int lbmutex_lock(lbmutex* pmutex);

int lbmutex_unlock(lbmutex* pmutex);

void lbmutex_close(lbmutex** ppmutex);
*/