#include "esp_log.h"
#include "esp_ota_ops.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_application/hardware_wifi.hpp"

#include "fri3d_private/flasher.hpp"
#include "fri3d_private/ota.hpp"

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

    // If we get here, enough of the system is initialized to persist the flash
    auto running = CFlasher::persist();

    auto nvs = this->getNvsManager().openSys();

    // We store the number in the name of the OTA partition because MicroPython can only read i32
    ESP_ERROR_CHECK(nvs_set_i32(nvs, "boot_partition", running == "ota_0" ? 0 : 1));
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

    ESP_LOGI(TAG, "Deactivated");
}

void COta::fetchFirmwares()
{
    if (!this->ensureWifi())
    {
        return;
    }

    if (!this->fetcher.refresh())
    {
        ESP_LOGE(TAG, "Could not fetch versions.");
    };
}

void COta::updateFirmware()
{
    if (!this->ensureWifi())
    {
        return;
    }

    for (const auto &item : this->selectedFirmware.images)
    {
        switch (item.first)
        {
        case CImage::Main:
            CFlasher::flash(item.second);
            break;
        case CImage::MicroPython:
            CFlasher::flash(item.second, "micropython");
            break;
        case CImage::RetroGoLauncher:
            CFlasher::flash(item.second, "launcher");
            break;
        case CImage::RetroGoCore:
            CFlasher::flash(item.second, "retro-core");
            break;
        case CImage::RetroGoPRBoom:
            CFlasher::flash(item.second, "prboom-go");
            break;
        case CImage::VFS:
            CFlasher::flash(item.second, "vfs");
            break;
        }
    }
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

    auto &firmwares = this->fetcher.getFirmwares(false);

    // Available versions
    if (firmwares.empty())
    {
        auto buttonFetchVersions = lv_button_create(versionsContainer);
        lv_obj_set_width(buttonFetchVersions, LV_PCT(80));
        lv_obj_add_event_cb(buttonFetchVersions, COta::onClickFetchVersions, LV_EVENT_CLICKED, this);

        auto labelFetchVersions = lv_label_create(buttonFetchVersions);
        lv_label_set_text(labelFetchVersions, "Fetch firmwares");
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

        for (const auto &version : firmwares)
        {
            lv_dropdown_add_option(dropDown, version.version.text.c_str(), LV_DROPDOWN_POS_LAST);
        }

        // Make sure the latest version is selected
        lv_dropdown_set_selected(dropDown, 0);
        this->selectedFirmware = firmwares[0];

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
    case OtaEvent::FetchFirmwares:
        this->fetchFirmwares();
        this->showVersions();
        break;
    case OtaEvent::SelectedFirmware:
        break;
    case OtaEvent::UpdateFirmware:
        this->updateFirmware();
        break;
    }
}

void COta::onClickFetchVersions(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->sendEvent({OtaEvent::FetchFirmwares});
}

void COta::onClickUpdate(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->sendEvent({OtaEvent::UpdateFirmware});
}

void COta::onVersionChange(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));
    auto dropDown = static_cast<lv_obj_t *>(lv_event_get_target(event));
    auto &versions = self->fetcher.getFirmwares(false);

    auto index = lv_dropdown_get_selected(dropDown);
    ESP_LOGI(TAG, "Selected version index: %lu", index);

    self->selectedFirmware = versions[index];
}

bool COta::ensureWifi()
{
    Application::Hardware::IWifi &wifi = this->getHardwareManager().getWifi();

    if (!wifi.getConnected())
    {
        wifi.connect();
    }

    bool result = wifi.waitOnConnect(30s, true);

    if (!result)
    {
        ESP_LOGE(TAG, "No wifi connection");
    }

    return result;
}

static COta ota_impl;
Application::CBaseApp &ota = ota_impl;

} // namespace Fri3d::Apps::Ota
