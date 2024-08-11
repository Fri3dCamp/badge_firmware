#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_application/hardware_wifi.hpp"

#include "fri3d_private/flasher.hpp"
#include "fri3d_private/ota.hpp"

namespace Fri3d::Apps::Ota
{

static const char *TAG = "Fri3d::Apps::Ota::COta";

static const char *NVS_MICROPYTHON = "microPython";
static const char *NVS_RETRO_GO_LAUNCHER = "rgLauncher";
static const char *NVS_RETRO_GO_CORE = "rgCore";
static const char *NVS_RETRO_GO_PRBOOM = "rgPRBoom";
static const char *NVS_VFS = "vfs";

COta::COta()
    : Application::CThread<OtaEvent>(TAG)
    , screen(nullptr)
    , showBeta(false)
    , updateMain(true)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
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

    // We store the number in the name of the OTA partition because MicroPython can only read i32
    auto nvs = this->getNvsManager().openSys();
    ESP_ERROR_CHECK(nvs_set_i32(nvs, "boot_partition", running == "ota_0" ? 0 : 1));

    // Fetch the current firmware versions
    // The main firmware version we get from the running image
    const esp_app_desc_t *app_desc = esp_app_get_description();
    this->currentVersions[CImage::Main] = CVersion(app_desc->version);
    this->currentFirmware = this->currentVersions[CImage::Main].simplify();

    // The other versions we fetch from NVS
    this->currentVersions[CImage::MicroPython] = CVersion(nvs.getString(NVS_MICROPYTHON).c_str());
    this->currentVersions[CImage::RetroGoLauncher] = CVersion(nvs.getString(NVS_RETRO_GO_LAUNCHER).c_str());
    this->currentVersions[CImage::RetroGoCore] = CVersion(nvs.getString(NVS_RETRO_GO_CORE).c_str());
    this->currentVersions[CImage::RetroGoPRBoom] = CVersion(nvs.getString(NVS_RETRO_GO_PRBOOM).c_str());
    this->currentVersions[CImage::VFS] = CVersion(nvs.getString(NVS_VFS).c_str());
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

    auto nvs = this->getNvsManager().openSys();

    for (const auto &item : this->selectedFirmware.images)
    {
        switch (item.first)
        {
        case CImage::Main:
            CFlasher::flash(item.second);
            break;
        case CImage::MicroPython:
            if (this->updateMicroPython && *this->updateMicroPython)
            {
                if (CFlasher::flash(item.second, "micropython"))
                {
                    ESP_ERROR_CHECK(nvs_set_str(nvs, NVS_MICROPYTHON, item.second.version.text.c_str()));
                }
            }
            break;
        case CImage::RetroGoLauncher:
            if (this->updateRetroGo && *this->updateRetroGo)
            {
                if (CFlasher::flash(item.second, "launcher"))
                {
                    ESP_ERROR_CHECK(nvs_set_str(nvs, NVS_RETRO_GO_LAUNCHER, item.second.version.text.c_str()));
                }
            }
            break;
        case CImage::RetroGoCore:
            if (this->updateRetroGo && *this->updateRetroGo)
            {
                if (CFlasher::flash(item.second, "retro-core"))
                {
                    ESP_ERROR_CHECK(nvs_set_str(nvs, NVS_RETRO_GO_CORE, item.second.version.text.c_str()));
                }
            }
            break;
        case CImage::RetroGoPRBoom:
            if (this->updateRetroGo && *this->updateRetroGo)
            {
                if (CFlasher::flash(item.second, "prboom-go"))
                {
                    ESP_ERROR_CHECK(nvs_set_str(nvs, NVS_RETRO_GO_PRBOOM, item.second.version.text.c_str()));
                }
            }
            break;
        case CImage::VFS:
            if (this->updateVfs && *this->updateVfs)
            {
                if (CFlasher::flash(item.second, "vfs"))
                {
                    ESP_ERROR_CHECK(nvs_set_str(nvs, NVS_VFS, item.second.version.text.c_str()));
                }
            }
            break;
        }
    }

    esp_restart();
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

            auto labelCurrentVersionTitle = lv_label_create(currentVersionContainer);
            lv_label_set_text(labelCurrentVersionTitle, firmwares.empty() ? "Current version" : "Current");
            lv_obj_set_style_text_decor(labelCurrentVersionTitle, LV_TEXT_DECOR_UNDERLINE, 0);

            auto labelCurrentVersion = lv_label_create(currentVersionContainer);
            lv_label_set_text(labelCurrentVersion, this->currentFirmware.text.c_str());
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
            this->sendEvent({OtaEvent::SelectedFirmware});

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
            lv_group_focus_obj(buttonUpdate);
            lv_obj_set_flex_grow(buttonUpdate, 1);
            lv_obj_add_event_cb(buttonUpdate, COta::onClickPreview, LV_EVENT_CLICKED, this);

            auto labelUpdate = lv_label_create(buttonUpdate);
            lv_label_set_text(labelUpdate, "Update");
            lv_obj_center(labelUpdate);
        }
    }

    lv_screen_load(this->screen);

    lv_unlock();
}

void COta::onImageCheckboxToggle(lv_event_t *event)
{
    auto &active = *static_cast<std::optional<bool> *>(lv_event_get_user_data(event));

    active = !*active;
}

void COta::addImageCheckbox(lv_obj_t *container, std::optional<bool> &active, CImage::ImageType imageType, bool enabled)
{
    auto checkbox = lv_checkbox_create(container);
    lv_checkbox_set_text(checkbox, CImage::typeToUIString.at(imageType).c_str());
    lv_obj_set_style_text_font(checkbox, &lv_font_montserrat_10, 10);
    lv_obj_add_event_cb(checkbox, onImageCheckboxToggle, LV_EVENT_CLICKED, &active);

    lv_state_t state = 0;
    if (*active)
    {
        state |= LV_STATE_CHECKED;
    }

    if (!enabled)
    {
        state |= LV_STATE_DISABLED;
    }

    lv_obj_add_state(checkbox, state);

    auto label = lv_label_create(container);
    lv_label_set_text_fmt(
        label,
        "%s -> %s",
        this->currentVersions[imageType].text.c_str(),
        this->selectedFirmware.images[imageType].version.text.c_str());
    lv_obj_set_width(label, LV_PCT(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_10, 0);
}

void COta::showUpdate()
{
    // Clear the screen
    this->hide();

    lv_lock();

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
        // Versions
        auto versionsContainer = lv_obj_create(container);
        lv_obj_set_size(versionsContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(versionsContainer, 1);
        lv_obj_set_flex_flow(versionsContainer, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(versionsContainer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        // TODO: Tight fit, but scrolling needs to be fixed first
        lv_obj_set_style_pad_row(versionsContainer, 1, 0);
        lv_obj_set_style_pad_all(versionsContainer, 3, 0);
        lv_obj_set_style_pad_left(versionsContainer, 10, 0);

        this->addImageCheckbox(versionsContainer, this->updateMain, CImage::Main, false);

        if (this->updateMicroPython)
        {
            this->addImageCheckbox(versionsContainer, this->updateMicroPython, CImage::MicroPython);
        }

        if (this->updateRetroGo)
        {
            this->addImageCheckbox(versionsContainer, this->updateRetroGo, CImage::RetroGoCore);
        }

        if (this->updateVfs)
        {
            this->addImageCheckbox(versionsContainer, this->updateVfs, CImage::VFS);
        }
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
            lv_obj_add_event_cb(buttonCancel, COta::onClickCancel, LV_EVENT_CLICKED, this);

            auto labelCancel = lv_label_create(buttonCancel);
            lv_label_set_text(labelCancel, "Cancel");
            lv_obj_center(labelCancel);
        }

        {
            // Update button
            auto buttonUpdate = lv_button_create(buttonsContainer);
            lv_group_focus_obj(buttonUpdate);
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
        // As normal users don't care about Retro-Go's inner workings, we bundle them and say they should update if any
        // of the images change
        {
            auto &images = this->selectedFirmware.images;

            // If the same firmware version is selected as the actively running image, we presume the user is trying to
            // recover from an error situation, so we just suggest flashing all.
            bool force = this->currentFirmware == this->selectedFirmware.version;

            // While by default we expect all images to be present in each firmware release (even if they are the same
            // version), we do allow them to be missing, mostly for possible future compatibility
            if (images.contains(CImage::MicroPython))
            {
                this->updateMicroPython =
                    force || images.at(CImage::MicroPython).version > this->currentVersions.at(CImage::MicroPython);
            }
            else
            {
                this->updateMicroPython = std::nullopt;
            }

            if (images.contains(CImage::RetroGoCore))
            {
                // We do expect at least the core image to be there if one of the other images change as it is used to
                // determine the version of Retro Go
                this->updateRetroGo =
                    force || images.at(CImage::RetroGoCore).version > this->currentVersions.at(CImage::RetroGoCore) ||
                    (images.contains(CImage::RetroGoLauncher) &&
                     images.at(CImage::RetroGoLauncher).version > this->currentVersions.at(CImage::RetroGoLauncher)) ||
                    (images.contains(CImage::RetroGoPRBoom) &&
                     images.at(CImage::RetroGoPRBoom).version > this->currentVersions.at(CImage::RetroGoPRBoom));
            }
            else
            {
                // No upgrade if no core
                this->updateRetroGo = std::nullopt;
            }

            if (images.contains(CImage::VFS))
            {
                this->updateVfs = force || images.at(CImage::VFS).version > this->currentVersions.at(CImage::VFS);
            }
            else
            {
                this->updateVfs = std::nullopt;
            }
        }
        break;
    case OtaEvent::UpdateFirmware:
        this->updateFirmware();
        break;
    case OtaEvent::UpdatePreview:
        this->showUpdate();
        break;
    case OtaEvent::CancelUpdate:
        this->showVersions();
        break;
    }
}

void COta::onClickFetchVersions(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->sendEvent({OtaEvent::FetchFirmwares});
}

void COta::onClickPreview(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->sendEvent({OtaEvent::UpdatePreview});
}

void COta::onVersionChange(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));
    auto dropDown = static_cast<lv_obj_t *>(lv_event_get_target(event));
    auto &versions = self->fetcher.getFirmwares(true);

    auto index = lv_dropdown_get_selected(dropDown);

    self->selectedFirmware = versions[index];
    self->sendEvent({OtaEvent::SelectedFirmware});
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

void COta::onClickCancel(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->sendEvent({OtaEvent::CancelUpdate});
}

void COta::onClickUpdate(lv_event_t *event)
{
    auto self = static_cast<COta *>(lv_event_get_user_data(event));

    self->sendEvent({OtaEvent::UpdateFirmware});
}

static COta ota_impl;
Application::CBaseApp &ota = ota_impl;

} // namespace Fri3d::Apps::Ota
