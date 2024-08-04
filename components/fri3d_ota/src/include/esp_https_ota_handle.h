#pragma once

#include "esp_https_ota.h"
#include "esp_ota_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

// Unfortunately, this is not in the public interface, so we need to check on every IDF update if it is still compatible
//
// While you're checking that, also make sure you check the VERY DIRTY HACK at the end of CFlasher::flash is still
// working.
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 2) && ESP_IDF_VERSION <= ESP_IDF_VERSION_VAL(5, 2, 2)
typedef enum
{
    ESP_HTTPS_OTA_INIT,
    ESP_HTTPS_OTA_BEGIN,
    ESP_HTTPS_OTA_IN_PROGRESS,
    ESP_HTTPS_OTA_SUCCESS,
} esp_https_ota_state;

struct esp_https_ota_handle
{
    esp_ota_handle_t update_handle;
    const esp_partition_t *update_partition;
    esp_http_client_handle_t http_client;
    char *ota_upgrade_buf;
    size_t ota_upgrade_buf_size;
    int binary_file_len;
    int image_length;
    int max_http_request_size;
    esp_https_ota_state state;
    bool bulk_flash_erase;
    bool partial_http_download;
    int max_authorization_retries;
#if CONFIG_ESP_HTTPS_OTA_DECRYPT_CB
    decrypt_cb_t decrypt_cb;
    void *decrypt_user_ctx;
    uint16_t enc_img_header_size;
#endif
};
#endif

#ifdef __cplusplus
}
#endif
