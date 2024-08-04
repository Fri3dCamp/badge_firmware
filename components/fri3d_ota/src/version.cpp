#include <algorithm>

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include "fri3d_application/lvgl/wait_dialog.hpp"
#include "fri3d_private/version.hpp"

namespace Fri3d::Apps::Ota
{

static const char *TAG = "Fri3d::Apps::Ota::CVersionFetcher";

CVersionFetcher::CVersionFetcher()
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

bool CVersionFetcher::refresh()
{
    Application::LVGL::CWaitDialog dialog("Fetching versions");
    dialog.show();

    this->versions = CVersions();

    auto buffer = CVersionFetcher::fetch();

    if (buffer.empty())
    {
        return false;
    }

    dialog.setStatus("Parsing version info");

    if (!this->parse(buffer.c_str()))
    {
        this->versions = CVersions();
    }

    return !this->versions.empty();
}

std::string CVersionFetcher::fetch()
{
    std::string buffer;

    esp_http_client_config_t config({});

    config.url = CONFIG_FRI3D_VERSIONS_URL;
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

const CVersions &CVersionFetcher::getVersions(bool beta) const
{
    return this->versions;
}

bool CVersionFetcher::parse(const char *json)
{
    cJSON *root = cJSON_Parse(json);

    if (root == nullptr)
    {
        ESP_LOGE(TAG, "Could not parse JSON");
        return false;
    }

    const cJSON *node;
    cJSON_ArrayForEach(node, root)
    {
        const char *version = cJSON_GetObjectItemCaseSensitive(node, "version")->valuestring;
        const char *url = cJSON_GetObjectItemCaseSensitive(node, "url")->valuestring;

        int size = -1;
        cJSON *size_json = cJSON_GetObjectItemCaseSensitive(node, "size");
        if (cJSON_IsNumber(size_json))
        {
            size = size_json->valueint;
        }

        CFirmwareVersion v = {CVersion(version), url, size};
        this->versions.push_back(v);
    }

    cJSON_Delete(root);

    std::sort(this->versions.begin(), this->versions.end());

    return true;
}

bool operator<(const CFirmwareVersion &l, const CFirmwareVersion &r)
{
    return l.version < r.version;
}

CVersion::CVersion(const char *version)
    : text(version)
    , semver({})
{
    auto result = semver_parse(this->text.c_str(), &this->semver);

    if (result != 0)
    {
        ESP_LOGW(TAG, "Could not parse version %s", version);
    }
}

CVersion::~CVersion()
{
    semver_free(&this->semver);
}

bool operator<(const CVersion &l, const CVersion &r)
{
    return semver_compare(l.semver, r.semver) == 1;
}

CVersion::CVersion()
    : CVersion("0.0.0")
{
}

} // namespace Fri3d::Apps::Ota
