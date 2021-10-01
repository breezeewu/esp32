#pragma once

int esp_http_client_post(const char* purl, const char* content_type, const char* req, int req_len, int* phttp_code, char* resp, int size);