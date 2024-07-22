#pragma once

#include "esp_event.h"
#include "esp_https_ota.h"

#ifdef __cplusplus
extern "C" {
#endif

void initialize_nvs(void);

void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void advanced_ota_example_task(void *pvParameters);

typedef struct upgrade_task_parameters_t
{
    const char* url;
    int size;
} upgrade_task_parameters_t;


#ifdef __cplusplus
}
#endif
