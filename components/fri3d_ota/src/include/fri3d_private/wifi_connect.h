#pragma once

#include "freertos/FreeRTOS.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "ota_wifi_secrets.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t example_connect(void);

#ifdef __cplusplus
}
#endif
