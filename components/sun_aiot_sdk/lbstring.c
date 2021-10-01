#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "lbstring.h"
#include "lbmacro.h"

const char* lbstring_read_line(const char* pstr, char* line, int line_len)
{
    int cp_len = 0;
    int str_len = strlen(pstr);
    const uint8_t* pnext = (uint8_t*)strchr(pstr, '\n');
    //lbinfo("pnext:%s = (uint8_t*)strchr(pstr:%s, \n)\n", pnext, pstr);
    memset(line, 0, line_len);
    if(NULL == pnext)
    {
        cp_len = str_len > line_len ? line_len - 1 : str_len;
        memcpy(line, pstr, cp_len);
        line[cp_len] = '\0';
        return NULL;
    }
    else
    {
        const uint8_t* ptmp = pnext;
        if('\r' == *(ptmp -1))
        {
            ptmp--;
        }
        cp_len = ptmp - (uint8_t*)pstr;
        cp_len = cp_len > line_len ? line_len - 1 : cp_len;
        memcpy(line, pstr, cp_len);
        //lbinfo("str_read_line, line:%s, pnext:%s", line, pnext);
        line[cp_len] = '\0';
        return (const char*)++pnext;
    }
}

const char* lbstring_read_tag(const char* pstr, const char* splt, char* tag, int tag_size)
{
    const char* pnext = NULL;
    const char* ptmp = NULL;
    int tag_len = 0;
    if(NULL == pstr || NULL == splt || NULL == tag || tag_size <= 0)
    {
        lberror("%s Invalid parameter, pstr:%s, splt:%s, tag:%s, tag_size:%d\n", __FUNCTION__, pstr, splt, tag, tag_size);
        return NULL;
    }
    memset(tag, 0, tag_size);
    pnext = strstr(pstr, splt);
    if(NULL == pnext)
    {
        tag_len = strlen(pstr);
    }
    else
    {
        tag_len = pnext - pstr;
        pnext += strlen(splt);
    }

    if(tag_len >= tag_size)
    {
        lberror("tag buffer size not enough, need:%d, have:%d\n", tag_len, tag_size);
        return NULL;
    }

    memcpy(tag, pstr, tag_len);
    //lbinfo("tag:%s, pnext:%s\n", tag, pnext);
    return pnext;
}

int lbstring_version_compare(const char* pver1, const char* pver2)
{
    int i = 0;
    int ret = -1;
    int ver1_dot_num = 0, ver2_dot_num = 0;
    char ver1[16];
    char ver2[16];
    char* pv1 = ver1, *pv2 = ver2;
    assert(pver1);
    assert(pver2);
    memset(ver1, 0, sizeof(ver1));
    memset(ver2, 0, sizeof(ver2));
    memcpy(ver1, pver1, strlen(pver1) + 1);
    memcpy(ver2, pver2, strlen(pver2) + 1);
    lbinfo("ver1:%s, pver1:%s, ver2:%s, pver2:%s\n", ver1, pver1, ver2, pver2);
    for(i = 0; i < strlen(pver1); i++)
    {
        if(ver1[i] == '.')
        {
            ver1_dot_num++;
            ver1[i] = '\0';
        }
    }

    if(ver1_dot_num < 2)
    {
        lberror("ver1_dot_num:%d, Invalid version1 string:%s\n", ver1_dot_num, pver1);
    }

    for(i = 0; i < strlen(pver2); i++)
    {
        if(ver2[i] == '.')
        {
            ver2_dot_num++;
            ver2[i] = '\0';
        }
    }

    if(ver2_dot_num < 2)
    {
        lberror("ver2_dot_num:%d, Invalid version2 string:%s\n", ver2_dot_num, pver2);
    }

    do{
        lbinfo("pv1:%p, pv2:%p\n", pv1, pv2);
        lbinfo("pv1:%s, pv2:%s\n", pv1, pv2);
        if(strlen(pv1) <= 0 && strlen(pv2) <= 0)
        {
            ret = 0;
            lbinfo("ver1:%s == ver2:%s, ret:%d\n", ver1, ver2, ret);
            break;
        }
        else if(strlen(pv1) <= 0)
        {
            ret = -1;
            lbinfo("ver1:%s < ver2:%s, ret:%d\n", ver1, ver2, ret);
            break;
        }
        else if(strlen(pv2) <= 0)
        {
            ret = 1;
            lbinfo("ver1:%s > ver2:%s, ret:%d\n", ver1, ver2, ret);
            break;
        }
        else
        {
            lbinfo("pv1:%p, pv2:%p\n", pv1, pv2);
            int v1 = atoi(pv1);
            int v2 = atoi(pv2);
            lbinfo("v1:%d, v2:%d\n", v1, v2);
            if(v1 == v2)
            {
                ret = 0;
                pv1 += strlen(pv1) + 1;
                pv2 += strlen(pv2) + 1;
                lbinfo("pv1:%s, pv2:%s\n", pv1, pv2);
            }
            else
            {
                if(v1 > v2)
                {
                    ret = 1;
                    lbinfo("v1:%d > v2:%d, ver1:%s > ver2:%s, ret:%d\n", v1, v2, ver1, ver2, ret);
                }
                else
                {
                    ret = -1;
                    lbinfo("v1:%d < v2:%d, ver1:%s < ver2:%s, ret:%d\n", v1, v2, ver1, ver2, ret);
                }
                break;
            }
        }
    }while(1);

    return ret;
}