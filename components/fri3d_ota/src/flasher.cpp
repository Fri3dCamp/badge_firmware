#include "esp_crt_bundle.h"
#include "esp_https_ota.h"
#include "esp_log.h"

// Declare ESP-IDF internal structs
#include "esp_https_ota_handle.h"

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

    // Find the partition
    if (partitionName != nullptr)
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
    Application::LVGL::CWaitDialog dialog("");
    dialog.show();

    bool result = false;

    if (partition == nullptr || partition->type == ESP_PARTITION_TYPE_APP)
    {
        result = CFlasher::flashOta(image, partition, httpConfig, dialog);
    }
    else
    {
        result = CFlasher::flashRaw(image, *partition, httpConfig, dialog);
    }

    dialog.setProgress(100.0f);

    return result;
}

bool CFlasher::flashOta(
    const CImage &image,
    const esp_partition_t *partition,
    esp_http_client_config_t &httpConfig,
    Application::LVGL::CWaitDialog &dialog)
{
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

    if (partition != nullptr)
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
    }

    setStatusFlashing(image, dialog);

    ESP_LOGI(TAG, "Starting OTA update on partition `%s`", ota->update_partition->label);

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

    if (partition != nullptr)
    {
        // THIS IS A VERY DIRTY HACK: by changing the state we trick `esp_https_ota_finish` to not write the next boot
        // partition
        //
        // This will only work as long as ESP_HTTPS_OTA_IN_PROGRESS does the same as ESP_HTTPS_OTA_SUCCESS except for
        // the boot partition step
        ota->state = ESP_HTTPS_OTA_IN_PROGRESS;
    }

    return (ESP_OK == esp_https_ota_finish(otaHandle));
}

bool CFlasher::flashRaw(
    const CImage &image,
    const esp_partition_t &partition,
    esp_http_client_config_t &httpConfig,
    Application::LVGL::CWaitDialog &dialog)
{
    RawUserData userData =
        {.partition = partition, .size = image.size, .contentLength = -2, .dialog = dialog, .written = 0};

    httpConfig.user_data = &userData;

    httpConfig.event_handler = [](esp_http_client_event_t *event) -> esp_err_t {
        RawUserData &data = *static_cast<RawUserData *>(event->user_data);

        switch (event->event_id)
        {
        case HTTP_EVENT_ERROR:
            ESP_LOGW(TAG, "HTTP_EVENT_ERROR");
            return ESP_FAIL;
            break;
        case HTTP_EVENT_ON_DATA:
            if (data.contentLength == -2)
            {
                // We haven't fetched the content length yet
                data.contentLength = static_cast<int>(esp_http_client_get_content_length(event->client));

                if (data.contentLength > 0)
                {
                    if (data.contentLength > data.size)
                    {
                        ESP_LOGE(
                            TAG,
                            "Server reported image size (%d) does not match metadata image size (%d)",
                            data.contentLength,
                            data.size);
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Server did not report image size.");
                    }
                }
            }

            if (ESP_OK != esp_partition_write(&data.partition, data.written, event->data, event->data_len))
            {
                ESP_LOGE(TAG, "Error while writing to flash");
                return ESP_FAIL;
            }

            data.written += event->data_len;
            {
                float progress = static_cast<float>(data.written) / static_cast<float>(data.size) * 100.0f;
                ESP_LOGI(TAG, "Image size: %d - bytes read: %d - progress: %03.2f", data.size, data.written, progress);
                data.dialog.setProgress(progress);
            }

            break;
        default:
            break;
        }
        return ESP_OK;
    };

    CFlasher::setStatusErasing(partition, dialog);
    auto step = (partition.size / 100 / partition.erase_size) * partition.erase_size;
    for (uint32_t i = 0; i <= partition.size; i += step)
    {
        esp_partition_erase_range(&partition, i, i + step);
        float progress = static_cast<float>(i) / static_cast<float>(partition.size) * 100.0f;
        ESP_LOGI(TAG, "Erasing `%s` %lu %lu - progress: %03.2f", partition.label, i, partition.size, progress);
        dialog.setProgress(progress);
    }

    CFlasher::setStatusFlashing(image, dialog);

    esp_http_client_handle_t client = esp_http_client_init(&httpConfig);

    ESP_LOGI(TAG, "Starting Raw OTA update on partition `%s`", partition.label);

    if (ESP_OK != esp_http_client_perform(client) || 200 != esp_http_client_get_status_code(client))
    {
        ESP_LOGE(TAG, "Could not download from %s", httpConfig.url);
        return false;
    }

    esp_http_client_cleanup(client);

    // httpConfig should no longer be used, but still, clear pointers to things that will disappear
    httpConfig.user_data = nullptr;
    httpConfig.event_handler = nullptr;

    return true;
}

void CFlasher::setStatusFlashing(const CImage &image, Application::LVGL::CWaitDialog &dialog)
{
    std::string dialogText = std::string("Flashing ") + CImage::typeToUIString.at(image.imageType) + "...";
    dialog.setStatus(dialogText.c_str());
}

void CFlasher::setStatusErasing(const esp_partition_t &partition, Application::LVGL::CWaitDialog &dialog)
{
    std::string dialogText = std::string("Erasing ") + partition.label + "...";
    dialog.setStatus(dialogText.c_str());
}

} // namespace Fri3d::Apps::Ota
