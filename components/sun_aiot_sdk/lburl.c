#include "lburl.h"
#include <stdio.h>
#include <string.h>
int lbstr_split(const char* url, const char* splt_tag, char* prefix, int prefix_len, char* next, int next_len)
{
    const char* ptmp = strstr(url, splt_tag);
    memset(prefix, 0, prefix_len);
    memset(next, 0, next_len);
    if(ptmp)
    {
        int pflen = ptmp - url;
        int nxlen = strlen(url) - pflen - strlen(splt_tag);
        if(pflen > prefix_len - 1 || pflen <= 0)
        {
            lberror("lbstr_split(url:%s, splt_tag：%s) failed, pflen:%d > prefix_len - 1:%d\n", url, splt_tag, pflen, prefix_len - 1);
            return -1;
        }
        if(nxlen > next_len - 1 || next_len <= 0)
        {
            lberror("lbstr_split(url:%s, splt_tag：%s) failed, nxlen:%d > next_len - 1:%d\n", url, splt_tag, nxlen, next_len - 1);
            return -1;
        }
        memcpy(next, ptmp + strlen(splt_tag), nxlen);
    }
    else
    {
        int url_len = strlen(url);
        if(url_len > next_len - 1)
        {
            lberror("lbstr_split(url:%s, splt_tag：%s) failed, url_len:%d > next_len - 1:%d\n", url, splt_tag, url_len, next_len - 1);
            return -1;
        }
        memcpy(next, url, url_len);
    }
    //lbinfo("lbstr_split(url:%s, splt_tag:%s, prefix:%s, prefix_len:%d, next:%s, next_len:%d) success", url, splt_tag, prefix, prefix_len, next, next_len);
    return 0;
}

lburl_context* lburl_context_open(const char* purl)
{
    lburl_context* puc = NULL;
    const char* ptmp = purl;
    char* param = NULL;
    const char* pb = purl, pe = NULL;
    char* url = NULL;//, eurl[256];
    int len = strlen(purl);
    int offset = 0;
    //lbinfo("lburl_context_open(%s)\n", purl);
    if(NULL == purl)
    {
        return NULL;
    }
    url = (char*)lbmalloc(MAX_URL_SIZE);
    memset(url, 0,  MAX_URL_SIZE);
    puc = (lburl_context*)lbmalloc(sizeof(lburl_context));
    memset(puc, 0, sizeof(lburl_context));
    lbstrcp(puc->purl, purl);

    // parser url param
    param = strchr(purl, '?');
    //lbinfo("param:%p = strchr(purl:%s, ?)\n", param, purl);
    if(param)
    {
        len = param - purl;
        param++;
        lbstrcp(puc->pparam, param);
    }
    memcpy(url, purl, len);
    //lbinfo("url:%s\n", url);
    //parser schema
    ptmp = strstr(url, "://");
    if(ptmp)
    {
        offset = ptmp - url;
        puc->pschema = (char*)lbmalloc(offset + 1);
        memcpy(puc->pschema, url, offset);
        puc->pschema[offset] = 0;
        offset += strlen("://");
        ptmp = strchr(url + offset, '/');
        if(ptmp)
        {
            int host_len = 0;
            char* pport_str = strchr(url + offset, ':');
            if(pport_str)
            {
                char port_str[100];
                int port_len = ptmp - url - offset;
                memcpy(port_str, pport_str, port_len);
                port_str[port_len] = 0;
                puc->port = atoi(port_str);
                host_len = pport_str - url - offset;
                //lbinfo("parser port:%d, port_str:%s, port_len:%d\n", puc->port, port_str, port_len);
            }
            else
            {
                char host[256];
                host_len = ptmp - url - offset;
            }

            puc->phost = (char*)lbmalloc(host_len + 1);
            memcpy(puc->phost, url + offset, host_len);
            puc->phost[host_len] = 0;
        }
    }
    else
    {
        ptmp = url;
    }
    //lbinfo("before puc->port:%d <= 0\n", puc->port);
    if(puc->port <= 0)
    {
        if(0 == memcmp("https", puc->pschema, strlen(puc->pschema)))
        {
            puc->port = 443;
        }
        else if(0 == memcmp("rtmp", puc->pschema, strlen(puc->pschema)))
        {
            puc->port = 1935;
        }
        else if(0 == memcmp("rtsp", puc->pschema, strlen(puc->pschema)))
        {
            puc->port = 554;
        }
        else
        {
            puc->port = 80;
        }
    }
    //lbinfo("before puc->ppath\n");
    puc->ppath = (char*)lbmalloc(strlen(ptmp) + 1);
    memcpy(puc->ppath, ptmp, strlen(ptmp) + 1);
    //lbinfo("before lbfree(url)\n");
    lbfree(url);
    //lbinfo("lburl_context_open(purl:%s) success, puc->schema:%s, puc->phost:%s, puc->port:%d, puc->ppath:%s, puc->pparam:%p\n", purl, puc->pschema, puc->phost, puc->port, puc->ppath, puc->pparam);
    return puc;
}

bool url_is_https(lburl_context* puc)
{
    return memcmp(puc->pschema, "https", strlen("https")) == 0;
}

void lburl_context_close(lburl_context** ppuc)
{
    if(ppuc && *ppuc)
    {
        //lbinfo("lburl_context_close(ppuc:%p) begin, *ppuc:%p\n", ppuc, *ppuc);
        lburl_context* puc = *ppuc;
        lbfree(puc->purl);
        lbfree(puc->pschema);
        lbfree(puc->phost);
        lbfree(puc->ppath);
        lbfree(puc->pparam);
        lbfree(puc);
        *ppuc = NULL;
        //lbinfo("lburl_context_close end\n");
    }
}

int lburl_parser_local_path(const char* path, char** dir, char** name, char** suffix)
{
    char* pdir = NULL;
    char* psuffix = NULL;
    const char* pname = NULL;
    int dir_len = 0, name_len = 0, suffix_len = 0;
    lbcheck_pointer(path, -1, "Invalid parameter, path:%s\n", path);
    //lbinfo("%s(path:%s, dir:%p, *dir:%p, name:%p, suffix:%p)\n", __FUNCTION__, path, dir, *dir, name, suffix);
    pname = strrchr(path, '/');
    if(NULL == pname)
    {
        pname = path;
    }
    
    psuffix = strrchr(pname, '.');
    dir_len = pname - path;
    name_len = psuffix ? psuffix - pname : strlen(pname);
    if(psuffix)
    {
        suffix_len = strlen(psuffix);
    }
    //lbinfo("1 dir:%p, *dir:%p\n", dir, *dir);
    if(pdir && dir_len > 0 && dir)
    {
        *dir = (char*)lbmalloc(dir_len + 1);
        memset(*dir, 0, dir_len+1);
        memcpy(*dir, path, dir_len);
        //lbinfo("path:%s, *dir:%s\n", path, *dir);
    }
    //lbinfo("2 dir:%p, *dir:%p\n", dir, *dir);
    if(pname && name_len > 0 && name)
    {
        char* tmp = (char*)lbmalloc(name_len + 1);
        memset(tmp, 0, name_len+1);
        memcpy(tmp, pname, name_len);
        *name = tmp;
        //lbinfo("*name:%s, pname:%s\n", *name, pname);
    }
    //lbinfo("3 dir:%p, *dir:%p\n", dir, *dir);
    //lbinfo("psuffix:%p && suffix_len:%d > 0 && suffix:%p\n", psuffix, suffix_len, suffix);
    if(psuffix && suffix_len > 0 && suffix)
    {
        char* tmp = (char*)lbmalloc(suffix_len + 1);
        //lbinfo("suffix_len:%d\n", suffix_len);
        //*suffix = (char*)lbmalloc(suffix_len + 1);
        memset(tmp, 0, suffix_len+1);
        //lbinfo("psuffix:%s\n", psuffix);
        memcpy(tmp, psuffix, suffix_len);
        *suffix = tmp;
        //lbinfo("*suffix:%s, psuffix:%s\n", *suffix, psuffix);
    }
    //lbinfo("4 dir:%p, *dir:%p\n", dir, *dir);
    //lbinfo("%s path:%s, *dir:%p,\n", __FUNCTION__, path, *dir);
    return 0;
}