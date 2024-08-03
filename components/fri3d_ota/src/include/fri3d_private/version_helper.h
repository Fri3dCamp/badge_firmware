#pragma once

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

cJSON *http_get_versions_task();

typedef struct http_rest_recv_buffer_t
{
    uint8_t *buffer;
    int buffer_len;
    int status_code;

} http_rest_recv_buffer_t;

typedef struct http_rest_recv_json_t
{
    cJSON *json;
    int status_code;

} http_rest_recv_json_t;

typedef struct ota_file_t
{
    char *name;
    char *url;
    int size;
} ota_file_t;

#ifdef __cplusplus
}
#endif
