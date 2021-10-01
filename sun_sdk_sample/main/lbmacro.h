#ifndef _LAZY_BEAR_MICRO_H_
#define _LAZY_BEAR_MICRO_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "lblog.h"
//#include "lbmemcheck.h"
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#ifndef lberror
//#include <srs_kernel_log.hpp>
#define lberror printf
#endif

#ifndef lbtrace
#define lbtrace printf
#endif

#ifndef lbinfo
#define lbinfo printf
#endif

/*#ifndef  lbmemory
#define lbmemory(level, ptr, len, fmt, ...)
//srs_trace_memory((const char*)ptr, len, ##__VA_ARGS__)
#endif*/
// free ptr of which create by malloc
#ifndef lbmalloc
#define lbmalloc(size) malloc(size);
#endif
#ifndef lbfree
#define lbfree(p) if(p) { free(p); p = NULL; }
#endif

#ifndef lbmem_add_ref
#define lbmem_add_ref(ptr, size)
#endif
#ifndef lbmem_rm_ref
#define lbmem_rm_ref(ptr)
#endif

#define h264_nal_type(pdata) (pdata)[0] & 0x1f
#define h265_nal_type(pdata) ((pdata)[0] & 0x7E)>>1

// malloc and copy string to pdst from psrc
#define lbstrcp(pdst, psrc) if(strlen(psrc) > 0) { pdst = (char*)lbmalloc(strlen(psrc) + 1); memcpy(pdst, psrc, strlen(psrc) + 1);}

// check if res is true, else break with error;
#ifndef lbcheck_break
#define lbcheck_break(res, pinfo, ...) if((res)) { lberror("[%s][%d] " pinfo " check result failed, break\n", __FILE__, __LINE__, ##__VA_ARGS__); break;}
#endif

// check if res is true, else return with error ret;
#ifndef lbcheck_return
#define lbcheck_return(ret, pinfo,  ...) if(0 != (ret)) { lberror("[%s][%d] " pinfo " check result failed, ret:%ld\n", __FILE__, __LINE__, ##__VA_ARGS__, (long)ret); return ret;}
#endif

// check if ptr is not null, else return with error ret;
#ifndef lbcheck_pointer
#define lbcheck_pointer(ptr, ret, pinfo,  ...) if(NULL == ptr) { lberror("[%s][%d] " pinfo " check pointer failed, ret:%ld\n", __FILE__, __LINE__,  ##__VA_ARGS__, (long)ret); return ret;}
#endif

// check if ptr is not null, else return with error ret;
#ifndef lbcheck_pointer_break
#define lbcheck_pointer_break(ptr, pinfo,  ...) if(NULL == ptr) { lberror("[%s][%d] " pinfo " check pointer failed, break\n", __FILE__, __LINE__,  ##__VA_ARGS__); break;}
#endif

// check if value is true, then return with error ret;
#ifndef lbcheck_value
#define lbcheck_value(val, ret, pinfo,  ...) if(val) { lberror("[%s][%d] " pinfo " check value failed, ret:%d\n", __FILE__, __LINE__,  ##__VA_ARGS__, (int)ret); return ret;}
#endif

// check if value is true, then return with error ret;
#ifndef lbcheck_value_break
#define lbcheck_value_break(val, pinfo,  ...) if(val) { lberror("[%s][%d] " pinfo " check value failed, break\n", __FILE__, __LINE__,  ##__VA_ARGS__); break;}
#endif

#define lbmax(a,b) ((a) > (b) ? (a) : (b))
#define lbmin(a,b) ((a) > (b) ? (b) : (a))

#define lbsleep(sec)        sleep(sec)
#define lbusleep(us)        usleep(us)

// delete ptr of which create by new
#define lbdel(ptr) if(ptr){ delete ptr; ptr = NULL; }

// delete array of which create by new []
#define lbdela(ptr) if(ptr) { delete[] ptr; ptr = NULL; }

// close file handle and put it null
#define lbfclosep(p) \
    if(p) { \
        fclose(p); \
        p = NULL; \
    } \
    (void)0

// open and write file with path
#define lbfwrite(pfile, pdata, len, path) \
    if(NULL == pfile) { \
        pfile = fopen(path, "wb"); \
    } \
    if(pfile) { \
        fwrite(pdata, 1, len, pfile); \
    } \
    (void)0

// open and read file with path
#define lbfread(pfile, pdata, len, rlen, path) \
    if(NULL == pfile) { \
        pfile = fopen(path, "rb"); \
    } \
    if(pfile) { \
        rlen = fread(pdata, 1, len, pfile); \
    } \
    (void)0

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

// version info
#define SUN_AIOT_COMMON_SDK_VERSION_MAJOR       0
#define SUN_AIOT_COMMON_SDK_VERSION_MINOR       2
#define SUN_AIOT_COMMON_SDK_VERSION_MACRO       0
#define SUN_AIOT_COMMON_SDK_VERSION_TINY        8


#endif