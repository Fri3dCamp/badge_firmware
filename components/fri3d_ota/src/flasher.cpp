#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"

// Declare ESP-IDF internal structs
#include "esp_https_ota_handle.h"

#include "fri3d_application/lvgl/wait_dialog.hpp"
#include "fri3d_private/flasher.hpp"

namespace Fri3d::Apps::Ota
{

static const char *TAG = "Fri3d::Apps::Ota::CFlasher";

CFlasher::CFlasher()
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

std::string CFlasher::persist()
{
    ESP_ERROR_CHECK(esp_ota_mark_app_valid_cancel_rollback());

    return esp_ota_get_running_partition()->label;
}

bool CFlasher::flash(const CImage &image)
{
    return CFlasher::flash(image, nullptr);
}

bool CFlasher::flash(const CImage &image, const char *partitionName)
{
    const esp_partition_t *partition = nullptr;
    bool customPartition = partitionName != nullptr;

    // Find the partition
    if (customPartition)
    {
        partition = esp_partition_find_first(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, partitionName);
        if (partition == nullptr)
        {
            ESP_LOGE(TAG, "Could not find partition %s", partitionName);
            return false;
        }
    }

    // Configure the HTTP client
    esp_http_client_config_t httpConfig({});

    httpConfig.url = image.url.c_str();
    httpConfig.timeout_ms = 5000;
    httpConfig.crt_bundle_attach = esp_crt_bundle_attach;
    httpConfig.keep_alive_enable = true;
    httpConfig.buffer_size_tx = 4096;
    httpConfig.buffer_size = 4096;

    // Show a dialog
    std::string dialogText = std::string("Flashing ") + CImage::typeToUIString.at(image.imageType) + "...";
    Application::LVGL::CWaitDialog dialog(dialogText.c_str());
    dialog.show();

    // Configure the OTA
    esp_https_ota_config_t otaConfig({});
    otaConfig.http_config = &httpConfig;

    esp_https_ota_handle_t otaHandle = nullptr;
    if (ESP_OK != esp_https_ota_begin(&otaConfig, &otaHandle))
    {
        ESP_LOGE(TAG, "Could not initialize OTA");
        return false;
    }

    // If you get a compile error here, it's probably because you updated IDF and didn't check and update
    // ./include/esp_https_ota_handle.h yet
    auto ota = static_cast<esp_https_ota_handle *>(otaHandle);

    if (customPartition)
    {
        ota->update_partition = partition;
    }
    else
    {
        // By default, IDF will switch to ota_2 after ota_1, which we don't want, so we reset it here
        if (ota->update_partition->subtype > ESP_PARTITION_SUBTYPE_APP_OTA_1)
        {
            ota->update_partition =
                esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, nullptr);
        }
        partitionName = ota->update_partition->label;
    }

    ESP_LOGI(TAG, "Starting OTA update on partition `%s`", partitionName);

    auto total = esp_https_ota_get_image_size(otaHandle);
    if (total != image.size)
    {
        ESP_LOGE(TAG, "Server reported image size (%d) does not match metadata image size (%d)", total, image.size);
    }

    while (ESP_ERR_HTTPS_OTA_IN_PROGRESS == esp_https_ota_perform(otaHandle))
    {
        auto read = esp_https_ota_get_image_len_read(otaHandle);
        float progress = static_cast<float>(read) / static_cast<float>(total) * 100.0f;
        ESP_LOGI(TAG, "Image size: %d - bytes read: %d - progress: %03.2f", total, read, progress);
        dialog.setProgress(progress);
    }

    if (!esp_https_ota_is_complete_data_received(otaHandle))
    {
        ESP_LOGE(TAG, "Complete data was not received.");
        esp_https_ota_abort(otaHandle);
        return false;
    }

    dialog.setProgress(100.0f);

    if (customPartition)
    {
        // THIS IS A VERY DIRTY HACK: by changing the state we trick `esp_https_ota_finish` to not write the next boot
        // partition, which we definitely do not want if we're flashing something like nvs or vfs
        //
        // This will only work as long as ESP_HTTPS_OTA_IN_PROGRESS does the same as ESP_HTTPS_OTA_SUCCESS except for
        // the boot partition step
        ota->state = ESP_HTTPS_OTA_IN_PROGRESS;
    }

    return (ESP_OK == esp_https_ota_finish(otaHandle));
}

} // namespace Fri3d::Apps::Ota
