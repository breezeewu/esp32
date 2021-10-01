/***************************************************************************************************************
 * filename     lazylog.h
 * describe     write log to console and local disk file
 * author       Created by dawson on 2019/04/18
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ***************************************************************************************************************/
#pragma once
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

// log level define
#define LOG_LEVEL_VERB            0x1
#define LOG_LEVEL_INFO            0x2
#define LOG_LEVEL_TRACE           0x3
#define LOG_LEVEL_WARN            0x4
#define LOG_LEVEL_ERROR           0x5
#define LOG_LEVEL_DISABLE         0x6

//log level name
#define LOG_LEVEL_VERB_NAME                     "verb"
#define LOG_LEVEL_INFO_NAME                     "info"
#define LOG_LEVEL_TRACE_NAME                    "trace"
#define LOG_LEVEL_WARN_NAME                     "warn"
#define LOG_LEVEL_ERROR_NAME                    "error"
static const char* verb_name   = "verb";
static const char* info_name   = "info";
static const char* trace_name  = "trace";
static const char* warn_name   = "warn";
static const char* error_name  = "error";
/*#define LOG_LEVEL_NAME_VERB         "verb"
#define LOG_LEVEL_NAME_INFO         "info"
#define LOG_LEVEL_NAME_TRACE        "trace"
#define LOG_LEVEL_NAME_WARN         "warn"
#define LOG_LEVEL_NAME_ERROR        "error"*/

// log output flag define
// console output log info
#define LOG_OUTPUT_FLAG_CONSOLE     0x1
// file output log info
#define LOG_OUTPUT_FLAG_FILE        0x2
#define MAX_LOG_SIZE 1024
#define check_pointer(ptr, ret)     if(NULL == ptr){return ret;}
/**
 * log context describe
 **/
typedef struct tlog_context
{
    // mutex
    pthread_mutex_t*    pmutex;

    // file descriptor
    FILE*       pfd;

    // log write size
    int64_t     llwrite_size;

    // log level
    int         nlog_level;

    // log output falg
    int         nlog_output_flag;

    // log write path
    char*       plog_path;
    char*       plog_dir;

    // log buffer
    char*       plog_buf;
    int         nlog_buf_size;

    // version info
    int         nmayjor_version;
    int         nminor_version;
    int         nmicro_version;
    int         ntiny_version;
    
    int		ngmtime;
}   log_ctx;

// log_ctx* g_plogctx; need to be define in cpp
extern log_ctx* g_plogctx;

// log micro define
//#define DISABLE_SV_LOG
#ifdef DISABLE_SV_LOG
#define lbinit_log(path, level, flag,  vmayjor, vminor, vmicro, vtiny)
#define lbverb(fmt, ...)
#define lbt_trace(tag, fmt, ...)
#define lbt_error(tag, fmt, ...)
#define lbinfo(fmt, ...)
#define lbtrace(fmt, ...)
#define lbwarn(fmt, ...)
#define lberror(fmt, ...)
#define lbmemory(level, pmem, size)
#else
#define lbinit_log(path, level, flag,  vmayjor, vminor, vmicro, vtiny)       g_plogctx = init_file_log(path, level, flag,  vmayjor, vminor, vmicro, vtiny)
#define lbverb(fmt, ...)               log_trace(g_plogctx, LOG_LEVEL_VERB, NULL, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define lbt_trace(tag, fmt, ...)         log_trace(g_plogctx, LOG_LEVEL_TRACE, tag, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define lbt_error(tag, fmt, ...)         log_trace(g_plogctx, LOG_LEVEL_ERROR, tag, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define lbinfo(fmt, ...)               log_trace(g_plogctx, LOG_LEVEL_INFO, NULL, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define lbtrace(fmt, ...)              log_trace(g_plogctx, LOG_LEVEL_TRACE, NULL, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define lbwarn(fmt, ...)               log_trace(g_plogctx, LOG_LEVEL_WARN, NULL, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define lbdebug(fmt, ...)              log_trace(g_plogctx, LOG_LEVEL_TRACE, NULL, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define lberror(fmt, ...)              log_trace(g_plogctx, LOG_LEVEL_ERROR, NULL, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define lbmemory(level, pmem, size, fmt, ...)    log_memory(g_plogctx, level,  NULL, __FILE__, __LINE__, __FUNCTION__, pmem, size, fmt, ##__VA_ARGS__);
#endif
#ifndef CHECK_RESULT
#define CHECK_RESULT(ret) if(ret < 0) { sv_error("check result failed, ret:%d\n", ret); return ret;}
#endif

#ifndef lbcheck_pointer
#define lbcheck_pointer(ptr, ret, pinfo,  ...) if(NULL == ptr) { lberror("check pointer failed, " pinfo " ret:%ld\n", __FILE__, __LINE__,  ##__VA_ARGS__, (long)ret); return ret;}
//#define lbcheck_pointer(ptr, ret, pinfo,  ...) if(NULL == ptr) { lberror("" pinfo " check pointer failed, ret:%d\n", (int)ret); return ret;}
//#define lbcheck_pointer(ptr, ret)     if(NULL == ptr){ sv_error("check pointer failed, ret:%d\n", ret); return ret;}
#endif
/**********************************************************************************************************
 * get local time zone
 * @return local time zone
 **********************************************************************************************************/
static int get_time_zone()
{
    int timezone = 0;
    time_t t1, t2;
    struct tm *tm_local, *tm_utc;
    time(&t1);
    t2 = t1;
    tm_local = localtime(&t1);
    t1 = mktime(tm_local);
    tm_utc = gmtime(&t2);
    t2 = mktime(tm_utc);
    timezone = (t1 - t2) / 60;
    return timezone;
}

/*
 * 函数名: VAVAHAL_GetNtpZoom
 * 函数功能: 读取时区偏移值(夏令时)
 * 返回值：时区值
 * 修改:
 * 备注:
 */
static int get_timezone()
{
  int time_zone;
  int tmp_zone;
  time_t time_utc;
  struct tm tm_gmt;
  
  // Get the UTC time
  time(&time_utc);
  struct tm tm_local;

  // Get the local time
  // Use localtime_r for threads safe
  localtime_r(&time_utc, &tm_local);

  // Change tm to time_t 
  //mktime(&tm_local);

  // Change it to GMT tm
  gmtime_r(&time_utc, &tm_gmt);

  time_zone = (tm_local.tm_hour * 3600 + tm_local.tm_min * 60 + tm_local.tm_sec) 
            - (tm_gmt.tm_hour * 3600 + tm_gmt.tm_min * 60 + tm_gmt.tm_sec);

  if(tm_local.tm_mday != tm_gmt.tm_mday)
  {
    tmp_zone = tm_local.tm_mday - tm_gmt.tm_mday;
    if(tmp_zone == 1 || tmp_zone < -25)
    {
      time_zone += 24 * 3600;
    }
    else if(tmp_zone == -1 || tmp_zone > 25)
    {
      time_zone -= 24 * 3600;
    }
  }

#if 0
  tmp_zone = tm_local.tm_hour - tm_gmt.tm_hour;
  if(tmp_zone < -12)
  {
    time_zone += 24 * 3600;
  }
  else if(tmp_zone >= 12 && tm_local.tm_mday != tm_gmt.tm_mday)
  {
    time_zone -= 24 * 3600;
  }
#endif

  //VAVAHAL_Print(LOG_LEVEL_DEBUG, "[%s][%d]: Timezone = %d\n", FUN, LINE, time_zone);

  return time_zone/60;
}
/*
 * 函数名: VAVAHAL_Gget_timezoneetNtpEx
 * 函数功能: 读取时区偏移值(min)
 * 返回值：时区值
 * 修改:
 * 备注:
 *
static int get_timezone()
{
    int time_zone;
    int tmp_zone;
    time_t time_utc;
    struct tm tm_gmt;
    
    // Get the UTC time
    time(&time_utc);
    struct tm tm_local;

    // Get the local time
    // Use localtime_r for threads safe
    localtime_r(&time_utc, &tm_local);

    // Change tm to time_t 
    //mktime(&tm_local);

    // Change it to GMT tm
    gmtime_r(&time_utc, &tm_gmt);

    time_zone = (tm_local.tm_hour * 60 + tm_local.tm_min) - (tm_gmt.tm_hour * 60 + tm_gmt.tm_min);
    printf("time_zone:%d\n", time_zone);
    if(tm_local.tm_mday != tm_gmt.tm_mday)
    {
        tmp_zone = tm_local.tm_mday - tm_gmt.tm_mday;
        if(tmp_zone == 1 || tmp_zone < -25)
        {
        time_zone += 24 * 3600;
        }
        else if(tmp_zone == -1 || tmp_zone > 25)
        {
        time_zone -= 24 * 3600;
        }
    }

    time_zone += tmp_zone;
    static int s_timezone = INT32_MIN;
    if(s_timezone == INT32_MIN)
    {
        s_timezone = time_zone;
    }
    else if(time_zone != s_timezone)
    {
        printf("get timezone failed, timezone:%d != s_timezone:%d\n", time_zone, s_timezone);
    }
    
    if(time_zone > 12*60 || time_zone < -12*60)
    {
        printf("Invalid timezeon:%d, org s_timezone:%d\n", time_zone, s_timezone);
        time_zone = s_timezone;
    }
    return time_zone;
}*/

/**********************************************************************************************************
 * create log context
 * @parma nlog_leve 日志输出等级, 
 * @param nlog_output_flag 日志输出标识, 1,输出到控制台，2输出到文件，3同时输出到控制台和文件
 * @param plog_path 日志输出路径
 * @return 日志上下文结构体
 **********************************************************************************************************/
static log_ctx* open_log_contex(const int nlog_leve, int nlog_output_flag, const char* plog_path)
{
    if(NULL == plog_path && (LOG_OUTPUT_FLAG_FILE & nlog_output_flag))
    {
        printf("plog_path:%p, nlog_output_flag:%d, failed!\n", plog_path, nlog_output_flag);
        return NULL;
    }

    // create log context
    log_ctx* plogctx = (log_ctx*)malloc(sizeof(log_ctx));
    check_pointer(plogctx, NULL);
    memset(plogctx, 0, sizeof(log_ctx));
    plogctx->nlog_level        = nlog_leve;

    // create and init mutex
    plogctx->pmutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(plogctx->pmutex, 0);

    // create and copy log path
    plogctx->plog_path         = (char*)malloc(strlen(plog_path)+1);
    strcpy(plogctx->plog_path, plog_path);
    plogctx->nlog_output_flag  = nlog_output_flag;

    // create log buffer
    plogctx->plog_buf          = (char*)malloc(MAX_LOG_SIZE);
    plogctx->nlog_buf_size     = MAX_LOG_SIZE;
    plogctx->ngmtime	       = 1;
    //printf("open_log_contex end, plogctx:%p\n", plogctx);
    return plogctx;
}

/**********************************************************************************************************
 * close log file and destroy log context
 * @parma pplogctx 日志上下文双重指针，执行成功后该指针将被置空
 **********************************************************************************************************/
static void close_log_contex(log_ctx** pplogctx)
{
    if(NULL == pplogctx || NULL == *pplogctx)
    {
        return ;
    }
    printf("\nclose log ctx ***************************************************************\n");
    //log_trace(g_plogctx, LOG_LEVEL_TRACE, NULL, __FILE__, __LINE__, __FUNCTION__, "close logctx:%p", pplogctx);
    //sv_trace("close logctx:%p", pplogctx);
    log_ctx* plogctx = *pplogctx;
    // lock the mutex before close file
    pthread_mutex_lock(plogctx->pmutex);
    if(plogctx->pfd)
    {
        fclose(plogctx->pfd);
        plogctx->pfd = NULL;
    }
    if(plogctx->plog_path)
    {
        free(plogctx->plog_path);
        plogctx->plog_path = NULL;
    }
    if(plogctx->plog_dir)
    {
        free(plogctx->plog_dir);
        plogctx->plog_dir = NULL;
    }
    if(plogctx->plog_buf)
    {
        free(plogctx->plog_buf);
        plogctx->plog_buf = NULL;
    }
    pthread_mutex_unlock(plogctx->pmutex);

    // destroy mutex last
    if(plogctx->pmutex)
    {
        pthread_mutex_destroy(plogctx->pmutex);
        free(plogctx->pmutex);
        plogctx->pmutex = NULL;
    }
    free(plogctx);
    plogctx = NULL;
    *pplogctx = NULL;
}

/**********************************************************************************************************
 * set current log version info
 * @parma:plogctx 日志上下文结构体
 * @param nmayjor  程序主版本号，程序主体架构有很大的调整时（如重构）变更
 * @param nminor   程序次版本号，程序功能和结构迭代时变更
 * @param nmicro   程序微版本号, 程序修改一些小bug和小幅度的功能调整时变更
 * @param ntiny    程序极小版本号，程序日志增删、同一个类型bug的不同修改时变更
 * @return 0为成功，否则为失败
 **********************************************************************************************************/
static int set_current_version(log_ctx* plogctx, const int nmayjor, const int nminor, const int nmicro, const int ntiny)
{
    if(NULL == plogctx)
    {
        return -1;
    }

    pthread_mutex_lock(plogctx->pmutex);
    plogctx->nmayjor_version   = nmayjor;
    plogctx->nminor_version    = nminor;
    plogctx->nmicro_version    = nmicro;
    plogctx->ntiny_version     = ntiny;
    pthread_mutex_unlock(plogctx->pmutex);

    return 0;
}

/**********************************************************************************************************
 * init write file log in a simple way
 * @param plogdir 日志输出路径
 * @param nloglevel 日志等级
 * @param nlogoutputflag 日志输出标识, 输出到文件，控制台
 * @param nmayjor   程序主版本号，程序主体架构有很大的调整时（如重构）变更
 * @param nminor    程序次版本号，程序功能和结构迭代时变更
 * @param nmicro    程序微版本号, 程序修改一些小bug和小幅度的功能调整时变更
 * @param ntiny     程序极小版本号，程序日志增删、同一个类型bug的不同修改时变更
 * @return 日志上下文结构体
 **********************************************************************************************************/
static log_ctx* init_file_log(const char* plogdir, const int nloglevel, const int nlogoutputflag,  const int nmayjor, const int nminor, const int nmicro, const int ntiny)
{
    char logpath[256] = {0};
    if(plogdir && (nlogoutputflag&LOG_OUTPUT_FLAG_FILE))
    {
        printf("if plogdir:%p\n", plogdir);
        if(access(plogdir, F_OK) != 0)
        {
            mkdir(plogdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            printf("plog_dir:%s\n", plogdir);
        }
        strcpy(logpath, plogdir);
        if(logpath[strlen(logpath)-1] != '/')
        {
            logpath[strlen(logpath)] = '/';
        }
        struct timeval tv;
        if (gettimeofday(&tv, NULL) == -1)
        {
            printf("gettimeofday failed\n");
            return NULL;
        }
        
        sprintf(&logpath[strlen(logpath)], "device_sdk_%ld.log", tv.tv_sec);
    }

    log_ctx* plogctx = open_log_contex(nloglevel, nlogoutputflag, logpath);
    printf("plogctx:%p = open_log_contex\n", plogctx);
    plogctx->plog_dir = (char*)malloc(strlen(plogdir)+1);
    strcpy(plogctx->plog_dir, plogdir);
    set_current_version(plogctx, nmayjor, nminor, nmicro, ntiny);
    printf("init_file_log end, plogctx:%p\n", plogctx);
    return plogctx;
}

/**********************************************************************************************************
 * create log line header with code file name, line, function name, thread id, process id, level, context_id
 * @parma plogctx 日志上下文结构体
 * @param plogbuf 用于接收头部信息的buffer
 * @param ptag  输出日志标签信息
 * @param pfilepath 输出日志的代码文件名
 * @param line  输出日志的代码位置（行数）
 * @param pfun  输出日志的函数名称
 * @param nlevel 输出日志的等级
 * @param pheader_size 头部信息的buffer大小，头部创建成功后返回头部信息大小
 * @return 0为成功，否则为失败
 **********************************************************************************************************/
static int generate_header(log_ctx* plogctx, char* plogbuf, const char* ptag,  const char* pfilepath, const int line, const char* pfun, const int nlevel, int* pheader_size)
{
    int log_header_size = -1;
    int log_buf_size;
    char loghdr[256] = {0};
    const char* pfilename = NULL;
    char levelname[20];
    struct timeval tv;
    struct tm* tm = NULL;
    if(NULL == plogbuf || NULL == pheader_size)
    {
        return -1;
    }

    log_buf_size = *pheader_size;
    // clock time
    //printf("generate_header begin\n");
    if (gettimeofday(&tv, NULL) == -1)
    {
        return -1;
    }

    //search the last '/' at pfile_path, return pos + 1
    pfilename = strrchr(pfilepath, '/');
    if(pfilename)
    {
        pfilename += 1;
    }
    else
    {
        pfilename = pfilepath;
    }
    //printf("pfilename:%s, pfile_path:%s\n", pfilename, pfile_path);
    if(ptag)
    {
        sprintf(loghdr, "%s %s:%d %s", ptag, pfilename, line, pfun);
    }
    else
    {
        sprintf(loghdr, "%s:%d %s", pfilename,line, pfun);
    }

    // use local time, if you want to use utc time, please use tm = gmtime(&tv.tv_sec)
    if(plogctx->ngmtime)
    {
        tm = gmtime(&tv.tv_sec);
        //printf("tm:%p = gmtime(&tv.tv_sec)\n", tm);
        if(!tm)
        {
            printf("tm:%p = gmtime(&tv.tv_sec) failed\n", tm);
            return -1;
        }
	//printf("after tm = gmtime(&tv.tv_sec)\n");
    }
    else if ((tm = localtime(&tv.tv_sec)) == NULL)
    {
	    printf("tm:%p = localtime(&tv.tv_sec) failed\n", tm);
        return -1;
    }

    // from level to log name
    switch(nlevel)
    {
        case LOG_LEVEL_VERB:
            memcpy(levelname, LOG_LEVEL_VERB_NAME, strlen(LOG_LEVEL_VERB_NAME) + 1);
            //plevelname = verb_name;
        break;
        case LOG_LEVEL_INFO:
            memcpy(levelname, LOG_LEVEL_INFO_NAME, strlen(LOG_LEVEL_INFO_NAME) + 1);
            //plevelname = info_name;
        break;
        case LOG_LEVEL_TRACE:
            memcpy(levelname, LOG_LEVEL_TRACE_NAME, strlen(LOG_LEVEL_TRACE_NAME) + 1);
            //plevelname = trace_name;
        break;
        case LOG_LEVEL_WARN:
            memcpy(levelname, LOG_LEVEL_WARN_NAME, strlen(LOG_LEVEL_WARN_NAME) + 1);
            //plevelname = warn_name;
        break;
        case LOG_LEVEL_ERROR:
            memcpy(levelname, LOG_LEVEL_ERROR_NAME, strlen(LOG_LEVEL_ERROR_NAME) + 1);
            //plevelname = error_name;
        break;
        case LOG_LEVEL_DISABLE:
        default:
            return 0;
    }

    // write log header
    if (nlevel >= LOG_LEVEL_ERROR && tm) {
        log_header_size = snprintf(plogbuf, log_buf_size, 
            "[%d-%02d-%02d %02d:%02d:%02d.%03d][%s][%s][%u][%d][%s] ",
            1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec / 1000), 
            levelname, loghdr, 0, errno, strerror(errno));
    } else {
        log_header_size = snprintf(plogbuf, log_buf_size, 
            "[%d-%02d-%02d %02d:%02d:%02d.%03d][%s][%s][%u] ",
            1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec / 1000), 
            levelname, loghdr, 0);
    }
    //printf("gen header end\n");
    if (log_header_size == -1) {
        return -1;
    }

    // write the header size.
    *pheader_size = (log_buf_size - 1) > log_header_size ? log_header_size : (log_buf_size - 1);
    //printf("generate_header end\n");
    return log_header_size;
}

/**********************************************************************************************************
 * write log to the output
 * @parma plogctx 日志上下文结构体
 * @param plogbuf 输出日志信息buffer
 * @param nloglen 输出日志信息长度
 * @return 0为成功，否则为失败
 **********************************************************************************************************/
static int write_log(log_ctx* plogctx, const int nlevel, char* plogbuf, const int nloglen)
{
    int loglen = nloglen;
    if(NULL == plogctx || NULL == plogbuf || nloglen <= 0 || 0 == plogctx->nlog_output_flag)
    {
        return -1;
    }

    loglen = (plogctx->nlog_buf_size - 1) > nloglen ? nloglen : (plogctx->nlog_buf_size - 1);

    // add some to the end of char.
    if('\n' != plogbuf[loglen-1])
    {
        plogbuf[loglen++] = '\n';
    }
    plogctx->llwrite_size += loglen;

    // if not to file, to console and return.
    if (LOG_OUTPUT_FLAG_CONSOLE & plogctx->nlog_output_flag)
    {
        // if is error msg, then print color msg.
        // \033[31m : red text code in shell
        // \033[32m : green text code in shell
        // \033[33m : yellow text code in shell
        // \033[0m : normal text code
        if (nlevel <= LOG_LEVEL_TRACE) {
            printf("%.*s", loglen, plogbuf);
        } else if (nlevel == LOG_LEVEL_WARN) {
            printf("\033[33m%.*s\033[0m", loglen, plogbuf);
        } else{
            printf("\033[31m%.*s\033[0m", loglen, plogbuf);
        }
        //fflush(stdout);
    }

    if((LOG_OUTPUT_FLAG_FILE & plogctx->nlog_output_flag) && plogctx->pfd)
    {
	    fwrite(plogbuf, 1, loglen, plogctx->pfd);
    }

    return 0;
}

/**********************************************************************************************************
 * get system version
 * @parma psysver buf to receive system version
 * @parms len buffer len
 * @return system version length if success, else fail 
 **********************************************************************************************************/
static int get_system_version(char* psysver, int len)
{
    FILE* pfile = fopen("/proc/sys/kernel/version", "r");
    int readlen = 0;
    if(pfile)
    {
        readlen = (int)fread(psysver, 1, 80, pfile);
        fclose(pfile);
        pfile = NULL;
    }

    return readlen;
}

/**********************************************************************************************************
 * log to the output
 * @parma plogctx 日志上下文结构体
 * @parms nlevel 日志输出等级
 * @param tag 日志输出标签
 * @param pfile 日志输出代码文件名称
 * @param line 日志输出代码位置（行数）
 * @param pfun 日志输出代码函数名称
 * @param fmt , ... 日志输出格式化字符串及其可变参数
 * @return 0为成功，否则为失败
**********************************************************************************************************/
static int log_trace(log_ctx* plogctx, const int nlevel, const char* tag, const char* pfile, int line, const char* pfun, const char* fmt, ...)
{
    int ret = -1;
    int nlog_buf_size = 0;
    check_pointer(plogctx, -1);
    if (plogctx->nlog_level > nlevel)
    {
        return 0;
    }
    //printf("log_trace begin\n");
    pthread_mutex_lock(plogctx->pmutex);
    //printf("log_trace after lock\n");
    nlog_buf_size = plogctx->nlog_buf_size;
    if (!generate_header(plogctx, plogctx->plog_buf, tag, pfile, line, pfun, nlevel, &nlog_buf_size))
    {
        pthread_mutex_unlock(plogctx->pmutex);
        return ret;
    }
    //printf("log_trace after generate_header\n");
    // format log info string
    va_list ap;
    va_start(ap, fmt);
    // we reserved 1 bytes for the new line.
    nlog_buf_size += vsnprintf(plogctx->plog_buf + nlog_buf_size, plogctx->nlog_buf_size - nlog_buf_size, fmt, ap);
    va_end(ap);
    //printf("log_trace after vsnprintf\n");
    if(NULL == plogctx->pfd && plogctx->plog_path && (LOG_OUTPUT_FLAG_FILE & plogctx->nlog_output_flag))
    {
		//printf("open log file\n");
		struct timeval tv;
		struct timezone tz;
        gettimeofday(&tv, &tz);
    	plogctx->pfd = fopen(plogctx->plog_path, "wb");
		printf("plogctx->pfd:%p = fopen(plogctx->plog_path:%s, wb)", plogctx->pfd, plogctx->plog_path);
		printf("after open log file:%p\n", plogctx->pfd);
        char loghdr[256] = {0};
        sprintf(loghdr, "version:%d.%d.%d.%d, timezone:%d, system version:", plogctx->nmayjor_version, plogctx->nminor_version, plogctx->nmicro_version, plogctx->ntiny_version, get_time_zone());
        get_system_version(loghdr + strlen(loghdr), 256 - (int)strlen(loghdr));

        write_log(plogctx, nlevel, loghdr, (int)strlen(loghdr));
    }
    //printf("log_trace after log open\n");
    ret = write_log(plogctx, nlevel, plogctx->plog_buf, nlog_buf_size);
    ///printf("log_trace after write_log\n");
    pthread_mutex_unlock(plogctx->pmutex);
    
    return ret;
}

/**********************************************************************************************************
 * log memory to the output
 * @parma plogctx 日志上下文结构体
 * @parms nlevel generate_header 日志输出等级
 * @param pmemory 输出内存指针
 * @param len 输出内存大小
 * @return 0为成功，否则为失败
 **********************************************************************************************************/
static int log_memory(log_ctx* plogctx, const int nlevel, const char* tag, const char* pfile, int line, const char* pfun, const char* pmemory, int len, const char* fmt, ...)
{
    int nlog_buf_size = 0;
    int i = 0;
    check_pointer(plogctx, -1);
    if(plogctx->nlog_level > nlevel)
    {
        return 0;
    }

    pthread_mutex_lock(plogctx->pmutex);
    nlog_buf_size = plogctx->nlog_buf_size;
    //printf("generate_header(plogctx:%p, plogctx->plog_buf:%p, tag:%p, pfile:%s, line:%d, pfun:%s, nlevel:%d, &nlog_buf_size:%d)\n", plogctx, plogctx->plog_buf, tag, pfile, line, pfun, nlevel, nlog_buf_size);
    if (!generate_header(plogctx, plogctx->plog_buf, tag, pfile, line, pfun, nlevel, &nlog_buf_size))
    {
        pthread_mutex_unlock(plogctx->pmutex);
        return -1;
    }
    //printf("generate_header end\n");
    va_list ap;
    va_start(ap, fmt);
    // we reserved 1 bytes for the new line.
    nlog_buf_size += vsnprintf(plogctx->plog_buf + nlog_buf_size, plogctx->nlog_buf_size - nlog_buf_size, fmt, ap);
    va_end(ap);

    //pthread_mutex_lock(plogctx->pmutex);
    const unsigned char* psrc = (unsigned char*)pmemory;
    //memset(plogctx->plog_buf, 0, plogctx->nlog_buf_size);
    //sprintf(plogctx->plog_buf, "[%s][%d][%s] size:%d, memory:", pfile, line, pfun, len);
    for(i = 0; i < len && (int)strlen(plogctx->plog_buf) < plogctx->nlog_buf_size; i++)
    {
        int val = *(psrc + i);
        sprintf(plogctx->plog_buf + strlen(plogctx->plog_buf), "%02x", val);
    }
    write_log(plogctx, nlevel, plogctx->plog_buf, (int)strlen(plogctx->plog_buf));
    pthread_mutex_unlock(plogctx->pmutex);
    return 0;
}

/**********************************************************************************************************
 * get current system time in microsecond
 * @return system current time in microseconds
**********************************************************************************************************/
static unsigned long lbget_sys_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long curtime = tv.tv_sec*1000 + tv.tv_usec/1000;
    return curtime;
}

static int create_if_not_exist(const char* pdir)
{
    if(access(pdir, F_OK) != 0)
    {
        mkdir(pdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        printf("mkdir:%s\n", pdir);
        return 0;
    }

    return 0;
}

/**********************************************************************************************************
 * get current log path dir
 * @return 成功返回日志路径指针常量，失败返回NULL
**********************************************************************************************************/
static const char* get_log_path()
{
    if(!g_plogctx)
    {
        return NULL;
    }
    
    return g_plogctx->plog_dir;
}

#define COMPARE_STRING(str1, str2)		0 ==  memcmp(str1, str2, strlen(str2))
#define VMPEAK	  	"VmPeak"
#define VMSIZE   	"VmSize"
#define VMHWM	  	"VmHWM"
#define VMRSS     	"VmRSS"
#define VMSTK		"VmStk"
#define VMEXE		"VmExe"
#define VMLIB		"VmLib"
#define VMDATA		"VmData"
static unsigned int get_proc_mem(int pid, int *vmpeak, int *vmem, int *mempeak, int *mem, int *mdata, int *stk, int *exe, int *lib)
{
	FILE *fd = NULL;
	//int line = 0;
	char name[64];
	char file_name[64];
	char line_buff[512];
	
	sprintf(file_name, "/proc/%d/status", pid);
	
	fd = fopen(file_name, "r");
	if(fd == NULL)
	{
		printf("open %s failed!", file_name);
		return 0;
	}

	while(1)
	{
		//line++;
		
		memset(line_buff, 0, 512);
		if(fgets(line_buff, 512, fd) == NULL)
		{
			break;
		}
		if(vmpeak && COMPARE_STRING(line_buff, VMPEAK))
		{
			
			sscanf(line_buff, "%s %d", name, vmpeak);
			//printf("%s:%d", VMPEAK, *vmpeak);
		}
		else if(vmem && COMPARE_STRING(line_buff, VMSIZE))
		{
			sscanf(line_buff, "%s %d", name, vmem);
		}
		else if(vmem && COMPARE_STRING(line_buff, VMHWM))
		{
			sscanf(line_buff, "%s %d", name, mempeak);
		}
		else if(vmem && COMPARE_STRING(line_buff, VMRSS))
		{
			sscanf(line_buff, "%s %d", name, mem);
		}
		else if(mdata && COMPARE_STRING(line_buff, VMDATA))
		{
			sscanf(line_buff, "%s %d", name, mdata);
		}
		else if(stk && COMPARE_STRING(line_buff, VMSTK))
		{
			sscanf(line_buff, "%s %d", name, stk);
		}
		else if(vmem && COMPARE_STRING(line_buff, VMEXE))
		{
			sscanf(line_buff, "%s %d", name, exe);
		}
		else if(lib && COMPARE_STRING(line_buff, VMLIB))
		{
			sscanf(line_buff, "%s %d", name, lib);
		}
	}

	fclose(fd);
	fd = NULL;
	
	return 0;
}
