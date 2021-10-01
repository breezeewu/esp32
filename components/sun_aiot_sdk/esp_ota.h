#pragma once
#include "sun_aiot_api.h"
int esp_init();
int esp_http_ota(const char* purl);

void* esp_http_create_ota_task(const char* purl);

int esp_http_ota_start_task(void* handle);
int esp_http_ota_get_progress(void* handle, e_aiot_http_msg_type* msg_type, http_download_info* phdp, int* percent);
int esp_http_ota_stop_task(void* handle);