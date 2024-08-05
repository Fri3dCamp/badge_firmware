#include <algorithm>

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include "fri3d_application/lvgl/wait_dialog.hpp"
#include "fri3d_private/firmware_fetcher.hpp"

namespace Fri3d::Apps::Ota
{

static const char *TAG = "Fri3d::Apps::Ota::CVersionFetcher";

CFirmwareFetcher::CFirmwareFetcher()
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

bool CFirmwareFetcher::refresh()
{
    Application::LVGL::CWaitDialog dialog("Fetching versions");
    dialog.show();

    this->firmwares = CFirmwares();

    auto buffer = CFirmwareFetcher::fetch();

    if (buffer.empty())
    {
        return false;
    }

    dialog.setStatus("Parsing version info");

    if (!this->parse(buffer.c_str()))
    {
        this->firmwares = CFirmwares();
    }

    return !this->firmwares.empty();
}

std::string CFirmwareFetcher::fetch()
{
    std::string buffer;

    esp_http_client_config_t config({});

    config.url = CONFIG_FRI3D_VERSIONS_URL;
    config.timeout_ms = 5000;
    config.user_data = &buffer;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.event_handler = [](esp_http_client_event_t *event) -> esp_err_t {
        std::string &buffer = *static_cast<std::string *>(event->user_data);

        switch (event->event_id)
        {
        case HTTP_EVENT_ERROR:
            ESP_LOGW(TAG, "HTTP_EVENT_ERROR");
            return ESP_FAIL;
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGV(TAG, "Received %d bytes", event->data_len);
            buffer.append(static_cast<const char *>(event->data), event->data_len);
            break;
        default:
            break;
        }
        return ESP_OK;
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (ESP_OK != esp_http_client_set_header(client, "Accept", "application/json"))
    {
        ESP_LOGE(TAG, "Could not set headers");
    }
    else
    {
        ESP_LOGD(TAG, "Downloading from %s", config.url);
        if (ESP_OK != esp_http_client_perform(client) || 200 != esp_http_client_get_status_code(client))
        {
            ESP_LOGE(TAG, "Could not download from %s", config.url);
            buffer = std::string();
        }
    }

    esp_http_client_cleanup(client);

    return buffer;
}

const CFirmwares &CFirmwareFetcher::getFirmwares(bool beta) const
{
    return this->firmwares;
}

bool CFirmwareFetcher::parse(const char *json)
{
    cJSON *root = cJSON_Parse(json);

    if (root == nullptr)
    {
        ESP_LOGE(TAG, "Could not parse JSON");
        return false;
    }

    const cJSON *firmwareNode;
    cJSON_ArrayForEach(firmwareNode, root)
    {
        cJSON *firmwareVersion = cJSON_GetObjectItemCaseSensitive(firmwareNode, "version");
        cJSON *images = cJSON_GetObjectItemCaseSensitive(firmwareNode, "images");
        cJSON *beta = cJSON_GetObjectItemCaseSensitive(firmwareNode, "beta");

        if (firmwareVersion == nullptr || images == nullptr || !cJSON_IsArray(images))
        {
            continue;
        }

        CFirmware firmware;
        firmware.version = CVersion(firmwareVersion->valuestring);
        firmware.beta = cJSON_IsTrue(beta);

        const cJSON *imageNode;
        cJSON_ArrayForEach(imageNode, images)
        {
            cJSON *imageType = cJSON_GetObjectItemCaseSensitive(imageNode, "type");
            cJSON *imageVersion = cJSON_GetObjectItemCaseSensitive(imageNode, "version");
            cJSON *url = cJSON_GetObjectItemCaseSensitive(imageNode, "url");
            cJSON *size = cJSON_GetObjectItemCaseSensitive(firmwareNode, "size");

            if (imageType == nullptr || !CImage::jsonStringToType.contains(imageType->valuestring) ||
                imageVersion == nullptr || url == nullptr)
            {
                continue;
            }

            CImage image;
            image.imageType = CImage::jsonStringToType.at(imageType->valuestring);
            image.version = CVersion(imageVersion->valuestring);
            image.url = url->valuestring;
            image.size = -1;

            if (cJSON_IsNumber(size))
            {
                image.size = size->valueint;
            }

            firmware.images[image.imageType] = image;
        }

        this->firmwares.emplace_back(firmware);
    }

    cJSON_Delete(root);

    std::sort(this->firmwares.begin(), this->firmwares.end());

    return true;
}

} // namespace Fri3d::Apps::Ota
