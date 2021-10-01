#pragma once
#include "lblist.h"


struct lbkv_list* lbkv_list_open();

int lbkv_list_add(struct lbkv_list* pkv_list, const char* pkey, const char* pvalue);

int lbkv_list_remove(struct lbkv_list* pkv_list, const char* pkey);

int lbkv_list_size(struct lbkv_list* pkv_list);

const char* lbkv_list_get_string_value(struct lbkv_list* pkv_list, const char* pkey);

int lbkv_list_get_long_value(struct lbkv_list* pkv_list, const char* pkey, long* pvalue);

void lbkv_list_close(struct lbkv_list** ppkv_list);