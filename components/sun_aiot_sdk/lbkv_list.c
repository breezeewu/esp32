#include<stdlib.h>
#include <string.h>
#include "lbmacro.h"
#include "lbkv_list.h"
typedef struct{
    char* pkey;
    char* pvalue;
} lbkv_pair;

struct lbkv_list{
    struct lblist_ctx* plist;
};

struct lbkv_list* lbkv_list_open()
{
    struct lbkv_list* pkv_list = (struct lbkv_list*)lbmalloc(sizeof(struct lbkv_list));
    memset(pkv_list, 0, sizeof(struct lbkv_list));
    pkv_list->plist = lblist_context_create(1000);
    //lbinfo("%s(), pkv_list:%p\n", __FUNCTION__, pkv_list);
    return pkv_list;
}
int lbkv_list_add(struct lbkv_list* pkv_list, const char* pkey, const char* pvalue)
{
    if(NULL == pkv_list || NULL == pkey || NULL == pvalue)
    {
        lberror("%s invalid parameter, pkv_list:%p, pkey:%s, pvalue:%s\n", pkv_list, pkey, pvalue);
        return -1;
    }
    //lbinfo("%s(pkv_list:%p, pkey:%s, pvalue:%s)\n", __FUNCTION__, pkv_list, pkey, pvalue);
    lbkv_list_remove(pkv_list, pkey);
    
    lbkv_pair* pkvp = (lbkv_pair*)lbmalloc(sizeof(lbkv_pair));
    lbstrcp(pkvp->pkey, pkey);
    lbstrcp(pkvp->pvalue, pvalue);

    return lblist_push(pkv_list->plist, pkvp);
}

int lbkv_list_remove(struct lbkv_list* pkv_list, const char* pkey)
{
    //lbinfo("%s(pkv_list:%p, pkey:%s)\n", __FUNCTION__, pkv_list, pkey);
    BEGIN_ENUM_LIST(pkv_list->plist, pnode)
    if(pnode && pnode->pitem)
    {
        lbkv_pair* pkvp = (lbkv_pair*)pnode->pitem;
        if(0 == memcmp(pkey, pkvp->pkey, strlen(pkey)))
        {
            lbfree(pkvp->pkey);
            lbfree(pkvp->pvalue);
            lbfree(pkvp);
            lblist_remove(pkv_list->plist, pnode);
            //lbinfo("lbkv_list_remove remove pkey:%s\n", pkey);
            return 0;
        }
    }
    END_ENUM_LIST;

    return -1;
}

int lbkv_list_size(struct lbkv_list* pkv_list)
{
    return lblist_size(pkv_list->plist);
}

const char* lbkv_list_get_string_value(struct lbkv_list* pkv_list, const char* pkey)
{
    //lbinfo("%s(pkv_list:%p, pkey:%s)\n", __FUNCTION__, pkv_list, pkey);
    BEGIN_ENUM_LIST(pkv_list->plist, pnode)
    if(pnode && pnode->pitem)
    {
        lbkv_pair* pkvp = (lbkv_pair*)pnode->pitem;
        if(0 == memcmp(pkey, pkvp->pkey, strlen(pkey)))
        {
           return pkvp->pvalue;
        }
    }
    END_ENUM_LIST;

    return 0;
}

int lbkv_list_get_long_value(struct lbkv_list* pkv_list, const char* pkey, long* pvalue)
{
    //lbinfo("%s(pkv_list:%p, pkey:%s, pvalue:%s)\n", __FUNCTION__, pkv_list, pkey, pvalue);
    BEGIN_ENUM_LIST(pkv_list->plist, pnode)
    if(pnode && pnode->pitem)
    {
        lbkv_pair* pkvp = (lbkv_pair*)pnode->pitem;
        if(0 == memcmp(pkey, pkvp->pkey, strlen(pkey)))
        {
            if(pvalue)
            {
                *pvalue = atol(pkvp->pvalue);
            }

            return 0;
        }
    }
    END_ENUM_LIST;

    return -1;
}

void lbkv_list_close(struct lbkv_list** ppkv_list)
{
    //lbinfo("%s(ppkv_list:%p)\n", __FUNCTION__, ppkv_list);
    if(ppkv_list && *ppkv_list)
    {
        struct lbkv_list* pkv_list = *ppkv_list;
        while(lblist_size(pkv_list->plist) > 0)
        {
            lbkv_pair* pkvp = (lbkv_pair*)lblist_pop(pkv_list->plist);
            lbfree(pkvp->pkey);
            lbfree(pkvp->pvalue);
            lbfree(pkvp);
        }
        lblist_context_close(&pkv_list->plist);
        lbfree(pkv_list);
    }

    //lbinfo("%s() end\n", __FUNCTION__);
}