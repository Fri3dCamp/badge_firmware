#include <algorithm>
#include <cstring>
#include <vector>

#include "esp_event.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_application/hardware_wifi.hpp"
#include "fri3d_application/lvgl/wait_dialog.hpp"

#include "fri3d_private/h2non_semver.h"
#include "fri3d_private/ota.hpp"
#include "fri3d_private/ota_helper.h"
#include "fri3d_private/version_helper.h"

namespace Fri3d::Apps::Ota
{

static const char *TAG = "Fri3d::Apps::Ota::COta";

COta::COta()
    : Application::CThread<OtaEvent>(TAG)
    , screen(nullptr)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));

    // Fetch the current firmware version
    const esp_app_desc_t *app_desc = esp_app_get_description();
    this->currentVersion = app_desc->version;
}

void COta::init()
{
    ESP_LOGI(TAG, "Initializing OTA update");
    if (this->screen == nullptr)
    {
        lv_lock();
        this->screen = lv_obj_create(nullptr);
        lv_unlock();
    }
}

void COta::deinit()
{
    ESP_LOGI(TAG, "Deinitializing OTA update");
    this->hide();

    if (this->screen != nullptr)
    {
        lv_lock();
        lv_obj_delete(this->screen);
        this->screen = nullptr;
        lv_unlock();
    }
}

const char *COta::getName() const
{
    return "OTA Update";
}

void COta::activate()
{
    this->start();
    this->showVersions();

    ESP_LOGI(TAG, "Activated");
}

void COta::deactivate()
{
    this->hide();
    this->stop();

    this->versions = CVersions();

    ESP_LOGI(TAG, "Deactivated");
}

void COta::do_fetch_versions()
{
    Application::Hardware::IWifi &wifi = this->getHardwareManager().getWifi();

    if (!wifi.getConnected())
    {
        wifi.connect();

        if (!wifi.waitOnConnect(30s, true))
        {
            return;
        }
    }

    Application::LVGL::CWaitDialog dialog("Fetching versions");
    dialog.show();

    cJSON *json = http_get_versions_task();

    dialog.setStatus("Parsing version info");

    ESP_LOGI(TAG, "tree_len: %d", cJSON_GetArraySize(json));

    this->versions = std::vector<ota_version_t>();

    const cJSON *node;
    cJSON_ArrayForEach(node, json)
    {
        char *version = cJSON_GetObjectItemCaseSensitive(node, "version")->valuestring;
        char *url = cJSON_GetObjectItemCaseSensitive(node, "url")->valuestring;

        int size = -1;
        cJSON *size_json = cJSON_GetObjectItemCaseSensitive(node, "size");
        if (cJSON_IsNumber(size_json))
        {
            size = size_json->valueint;
        }
        ota_version_t v = {version, url, size};
        this->versions.push_back(v);
    }

    // free the json memory that got allocated in http_get_versions_task
    if (json != NULL)
    {
        cJSON_Delete(json);
    }

    std::sort(this->versions.begin(), this->versions.end(), [](const ota_version_t &a, const ota_version_t &b) {
        semver_t s_a = {};
        semver_t s_b = {};
        int r_a = semver_parse(a.version.c_str(), &s_a);
        int r_b = semver_parse(b.version.c_str(), &s_b);
        bool result = false;
        if (r_a == 0 && r_b == 0)
        {
            result = semver_compare(s_a, s_b) == 1;
            semver_free(&s_a);
            semver_free(&s_b);
        }
        return result;
    });
}

void COta::do_upgrade()
{
    Application::Hardware::IWifi &wifi = this->getHardwareManager().getWifi();

    if (!wifi.getConnected())
    {
        wifi.connect();

        if (!wifi.waitOnConnect(30s, true))
        {
            return;
        }
    }

    Application::LVGL::CWaitDialog dialog("Upgrading...");
    dialog.show();

    ESP_ERROR_CHECK(esp_event_handler_register(ESP_HTTPS_OTA_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_LOGD(TAG, "this->selected_version.version: %s", this->selectedVersion.version.c_str());

    advanced_ota_example_task(this->selectedVersion.url.c_str(), this->selectedVersion.size);
}

bool COta::getVisible() const
{
    return true;
}

void COta::hide()
{
    lv_lock();
    lv_obj_clean(this->screen);
    lv_unlock();
}

void COta::showVersions()
{
    // Make sure the screen is clean in case we come back from the update screen
    this->hide();

    lv_lock();

    // Create a container with ROW flex direction
    auto versionsContainer = lv_obj_create(this->screen);
    lv_obj_remove_style_all(versionsContainer);
    lv_obj_set_size(versionsContainer, LV_PCT(80), LV_PCT(98));
    lv_obj_center(versionsContainer);
    lv_obj_set_flex_flow(versionsContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(versionsContainer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(versionsContainer, 10, 0);

    // Title
    auto labelTitle = lv_label_create(versionsContainer);
    lv_label_set_text(labelTitle, "OTA Update");

    // Current Version
    auto currentVersionContainer = lv_obj_create(versionsContainer);
    lv_obj_set_size(currentVersionContainer, LV_PCT(100), LV_SIZE_CONTENT);

    auto labelCurrentVersionTitle = lv_label_create(currentVersionContainer);
    lv_label_set_text(labelCurrentVersionTitle, "Firmware version");
    lv_obj_set_style_text_decor(labelCurrentVersionTitle, LV_TEXT_DECOR_UNDERLINE, 0);
    lv_obj_align(labelCurrentVersionTitle, LV_ALIGN_TOP_MID, 0, 0);

    auto labelCurrentVersion = lv_label_create(currentVersionContainer);
    lv_label_set_text(labelCurrentVersion, this->currentVersion);
    lv_obj_align_to(labelCurrentVersion, labelCurrentVersionTitle, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // Available versions
    if (this->versions.empty())
    {
        auto buttonFetchVersions = lv_button_create(versionsContainer);
        lv_obj_set_width(buttonFetchVersions, LV_PCT(80));
        lv_obj_add_event_cb(buttonFetchVersions, COta::onClickFetchVersions, LV_EVENT_CLICKED, this);

        auto labelFetchVersions = lv_label_create(buttonFetchVersions);
        lv_label_set_text(labelFetchVersions, "Fetch versions");
        lv_obj_center(labelFetchVersions);
    }
    else
    {
        // Dropdown to select a version
        auto dropDown = lv_dropdown_create(versionsContainer);
        lv_obj_set_width(dropDown, lv_pct(90));
        lv_obj_add_event_cb(dropDown, COta::onVersionChange, LV_EVENT_VALUE_CHANGED, this);

        // Add options
        lv_dropdown_clear_options(dropDown);

        for (const auto &version : this->versions)
        {
            lv_dropdown_add_option(dropDown, version.version.c_str(), LV_DROPDOWN_POS_LAST);
        }

        // Make sure the latest version is selected
        lv_dropdown_set_selected(dropDown, 0);
        this->selectedVersion = this->versions[0];

        // Update button
        auto buttonUpdate = lv_button_create(versionsContainer);
        lv_obj_set_width(buttonUpdate, LV_PCT(80));
        lv_obj_add_event_cb(buttonUpdate, COta::onClickUpdate, LV_EVENT_CLICKED, this);

        auto labelUpdate = lv_label_create(buttonUpdate);
        lv_label_set_text(labelUpdate, "Update firmware");
        lv_obj_center(labelUpdate);
    }

    // Cancel button
    auto buttonCancel = lv_button_create(versionsContainer);
    lv_obj_set_width(buttonCancel, LV_PCT(80));
    lv_obj_add_event_cb(buttonCancel, COta::onClickExit, LV_EVENT_CLICKED, this);

    auto labelCancel = lv_label_create(buttonCancel);
    lv_label_set_text(labelCancel, "Cancel");
    lv_obj_center(labelCancel);

    // Fill the empty space
    auto fill = lv_obj_create(versionsContainer);
    lv_obj_remove_style_all(fill);
    lv_obj_set_flex_grow(fill, 1);

    lv_screen_load(this->screen);

    lv_unlock();
}

void COta::onClickExit(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->getAppManager().previousApp();
}

void COta::onEvent(const OtaEvent &event)
{
    switch (event.eventType)
    {
    case OtaEvent::Shutdown:
        break;
    case OtaEvent::FetchVersions:
        this->do_fetch_versions();
        this->showVersions();
        break;
    case OtaEvent::SelectedVersion:
        break;
    case OtaEvent::Update:
        this->do_upgrade();
        break;
    }
}

void COta::onClickFetchVersions(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->sendEvent({OtaEvent::FetchVersions});
}

void COta::onClickUpdate(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->sendEvent({OtaEvent::Update});
}

void COta::onVersionChange(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));
    auto dropDown = static_cast<lv_obj_t *>(lv_event_get_target(event));

    auto index = lv_dropdown_get_selected(dropDown);
    ESP_LOGI(TAG, "Selected version index: %lu", index);

    self->selectedVersion = self->versions[index];
}

static COta ota_impl;
Application::CBaseApp &ota = ota_impl;

} // namespace Fri3d::Apps::Ota
