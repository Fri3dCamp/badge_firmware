#pragma once

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif


void http_get_versions_task(void *pvParameters);


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
    char* name;
    char* url;
    int size;
} ota_file_t;



typedef struct version_task_parameters_t
{
    const char* board_name;
    cJSON *json;
} version_task_parameters_t;

// int ota_version_qsort_d(const void* a, const void* b);
// int ota_version_qsort_a(const void* a, const void* b);

#ifdef __cplusplus
}
#endif