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
    , showBeta(false)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));

    // Fetch the current firmware version
    const esp_app_desc_t *app_desc = esp_app_get_description();
    this->currentVersion = CVersion(app_desc->version);
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
    // Make sure the screen is clean
    this->hide();

    lv_lock();

    auto &firmwares = this->fetcher.getFirmwares(this->showBeta);

    // Vertical flex container
    auto container = lv_obj_create(this->screen);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, LV_PCT(80), LV_PCT(90));
    lv_obj_center(container);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(container, 10, 0);

    {
        // Title
        auto labelTitle = lv_label_create(container);
        lv_label_set_text(labelTitle, "OTA Update");
    }

    {
        // Horizontal flex container
        auto versionContainer = lv_obj_create(container);
        lv_obj_remove_style_all(versionContainer);
        lv_obj_set_size(versionContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_center(versionContainer);
        lv_obj_set_flex_flow(versionContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(versionContainer, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

        {
            // Current Version
            // We wrap to align with the selectVersionContainer below
            auto wrapper = lv_obj_create(versionContainer);
            lv_obj_remove_style_all(wrapper);
            lv_obj_set_style_pad_all(wrapper, 3, 0);
            lv_obj_set_flex_grow(wrapper, 1);
            lv_obj_set_height(wrapper, LV_SIZE_CONTENT);

            auto currentVersionContainer = lv_obj_create(wrapper);
            lv_obj_set_size(currentVersionContainer, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(currentVersionContainer, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                currentVersionContainer,
                LV_FLEX_ALIGN_SPACE_EVENLY,
                LV_FLEX_ALIGN_CENTER,
                LV_FLEX_ALIGN_CENTER);
            //            lv_obj_set_style_pad_column(currentVersionContainer, 10, 0);

            auto labelCurrentVersionTitle = lv_label_create(currentVersionContainer);
            lv_label_set_text(labelCurrentVersionTitle, firmwares.empty() ? "Current version" : "Current");
            lv_obj_set_style_text_decor(labelCurrentVersionTitle, LV_TEXT_DECOR_UNDERLINE, 0);

            auto labelCurrentVersion = lv_label_create(currentVersionContainer);
            lv_label_set_text(labelCurrentVersion, this->currentVersion.simplify().text.c_str());
        }

        if (!firmwares.empty())
        {
            // Dropdown to select a version
            auto selectVersionContainer = lv_obj_create(versionContainer);
            lv_obj_remove_style_all(selectVersionContainer);
            // We have to add some padding otherwise the outline on the dropdown does not show
            lv_obj_set_style_pad_all(selectVersionContainer, 3, 0);
            lv_obj_set_flex_grow(selectVersionContainer, 1);
            lv_obj_set_height(selectVersionContainer, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(selectVersionContainer, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                selectVersionContainer,
                LV_FLEX_ALIGN_SPACE_EVENLY,
                LV_FLEX_ALIGN_CENTER,
                LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_row(selectVersionContainer, 10, 0);

            auto dropDown = lv_dropdown_create(selectVersionContainer);
            lv_obj_set_width(dropDown, LV_PCT(100));
            lv_obj_center(dropDown);
            lv_obj_add_event_cb(dropDown, COta::onVersionChange, LV_EVENT_VALUE_CHANGED, this);
            lv_dropdown_clear_options(dropDown);

            for (const auto &version : firmwares)
            {
                lv_dropdown_add_option(dropDown, version.version.text.c_str(), LV_DROPDOWN_POS_LAST);
            }

            // Make sure the latest version is selected
            lv_dropdown_set_selected(dropDown, 0);
            this->selectedFirmware = firmwares[0];

            auto checkboxBeta = lv_checkbox_create(selectVersionContainer);
            lv_checkbox_set_text(checkboxBeta, "Show Beta");
            if (this->showBeta)
            {
                lv_obj_add_state(checkboxBeta, LV_STATE_CHECKED);
            }
            lv_obj_add_event_cb(checkboxBeta, onCheckboxBetaToggle, LV_EVENT_CLICKED, this);
        }
    }

    {
        // Fill the empty space
        auto fill = lv_obj_create(container);
        lv_obj_remove_style_all(fill);
        lv_obj_set_flex_grow(fill, 1);
    }

    {
        // Buttons

        // Horizontal flex container
        auto buttonsContainer = lv_obj_create(container);
        lv_obj_remove_style_all(buttonsContainer);
        lv_obj_set_style_pad_all(buttonsContainer, 3, 0);
        lv_obj_set_size(buttonsContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_center(buttonsContainer);
        lv_obj_set_flex_flow(buttonsContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(buttonsContainer, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(buttonsContainer, 5, 0);

        {
            // Cancel button
            auto buttonCancel = lv_button_create(buttonsContainer);
            lv_obj_set_style_bg_color(buttonCancel, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_PART_MAIN);
            lv_obj_set_flex_grow(buttonCancel, 1);
            lv_obj_add_event_cb(buttonCancel, COta::onClickExit, LV_EVENT_CLICKED, this);

            auto labelCancel = lv_label_create(buttonCancel);
            lv_label_set_text(labelCancel, "Cancel");
            lv_obj_center(labelCancel);
        }

        if (firmwares.empty())
        {
            // Check online button
            auto buttonFetchVersions = lv_button_create(buttonsContainer);
            lv_group_focus_obj(buttonFetchVersions);
            lv_obj_set_flex_grow(buttonFetchVersions, 1);
            lv_obj_add_event_cb(buttonFetchVersions, COta::onClickFetchVersions, LV_EVENT_CLICKED, this);

            auto labelFetchVersions = lv_label_create(buttonFetchVersions);
            lv_label_set_text(labelFetchVersions, "Check");
            lv_obj_center(labelFetchVersions);
        }
        else
        {
            // Update button
            auto buttonUpdate = lv_button_create(buttonsContainer);
            lv_obj_set_flex_grow(buttonUpdate, 1);
            lv_obj_add_event_cb(buttonUpdate, COta::onClickUpdate, LV_EVENT_CLICKED, this);

            auto labelUpdate = lv_label_create(buttonUpdate);
            lv_label_set_text(labelUpdate, "Update");
            lv_obj_center(labelUpdate);
        }
    }

    lv_screen_load(this->screen);

    lv_unlock();
}

void COta::showUpdate()
{
    // Clear the screen
    this->hide();
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
    case OtaEvent::ToggleBeta:
        this->showBeta = !this->showBeta;
        this->showVersions();
        break;
    case OtaEvent::SelectedFirmware:
        break;
    case OtaEvent::UpdateFirmware:
        this->updateFirmware();
        break;
    case OtaEvent::UpdatePreview:
        this->showUpdate();
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

    self->sendEvent({OtaEvent::UpdatePreview});
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

void COta::onCheckboxBetaToggle(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->sendEvent({OtaEvent::ToggleBeta});
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
