#include <algorithm>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "esp_crt_bundle.h"
#include "esp_event.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "sdkconfig.h"

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
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void COta::init()
{
    ESP_LOGI(TAG, "Initializing ota");
}

void COta::deinit()
{
    ESP_LOGI(TAG, "Deinitializing ota");
}

const char *COta::getName() const
{
    return "Ota";
}

void COta::activate()
{
    ESP_LOGI(TAG, "Activated");

    const esp_app_desc_t *app_desc = esp_app_get_description();
    this->current_version = app_desc->version;

    this->screen_initial_layout();
    this->action = NoAction;

    while (true)
    {
        if (this->action == NoAction)
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        else if (this->action == CheckOnline)
        {
            this->do_fetch_versions();
            this->action = NoAction;
        }
        else if (this->action == Upgrade)
        {
            this->do_upgrade();
            this->action = NoAction;
        }
        else if (this->action == Cancel)
        {
            break;
        }
        else
        {
            ESP_LOGE(TAG, "unkown action: %d", this->action);
        }
    }

    this->getAppManager().previousApp();
}

void COta::deactivate()
{
    ESP_LOGI(TAG, "Deactivated");

    lv_lock();
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);
    this->label_error = nullptr;
    lv_unlock();
}

void COta::do_fetch_versions(void)
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

    // this should exist while the task is running -> class member (or wait till the task is finished here)
    // the whole version_task_parameters struct is overkill now, because board_name is not used any more
    // could just use const cJSON *json
    this->version_task_parameters = {};
    this->version_task_parameters.board_name = this->getBoardName();
    this->version_task_parameters.json = nullptr;
    ESP_LOGI(TAG, "board_name: %s", this->version_task_parameters.board_name);

    xTaskCreatePinnedToCore(
        &http_get_versions_task,
        "http_test_task",
        8192,
        &this->version_task_parameters,
        5,
        NULL,
        0);

    // http_get_versions_task(&this->version_task_parameters);

    // TODO: is this a good way to wait for the results?
    while (this->version_task_parameters.json == nullptr)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    dialog.setStatus("Parsing version info");

    const cJSON *versions = this->version_task_parameters.json;
    ESP_LOGI(TAG, "tree_len: %d", cJSON_GetArraySize(versions));

    this->available_versions = std::vector<ota_version_t>();

    const cJSON *node;
    cJSON_ArrayForEach(node, versions)
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
        this->available_versions.push_back(v);
    }

    // free the json memory that got allocated in http_get_versions_task
    if (this->version_task_parameters.json != NULL)
    {
        cJSON_Delete(this->version_task_parameters.json);
    }

    std::sort(
        this->available_versions.begin(),
        this->available_versions.end(),
        [](const ota_version_t &a, const ota_version_t &b) {
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

    this->drop_down_options = "";
    ESP_LOGI(TAG, "this->available_versions.size(): %" PRIu16, this->available_versions.size());
    for (size_t i = 0; i < this->available_versions.size(); i++)
    {
        ESP_LOGI(TAG, "this->available_versions[%" PRIu16 "]: %s", i, this->available_versions[i].version.c_str());
        this->drop_down_options += this->available_versions[i].version;
        if (i < this->available_versions.size() - 1)
            this->drop_down_options += "\n";
    }
    ESP_LOGD(TAG, "drop_down_options: %s", this->drop_down_options.c_str());

    this->screen_update_available_versions();
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

    ESP_LOGD(TAG, "this->selected_version.version: %s", this->selected_version.version.c_str());
    this->upgrade_task_parameters = {};
    this->upgrade_task_parameters.url = this->selected_version.url.c_str();
    this->upgrade_task_parameters.size = this->selected_version.size;

    advanced_ota_example_task(&this->upgrade_task_parameters);
}

const char *COta::getBoardName()
{
#ifdef CONFIG_FRI3D_BADGE_OCTOPUS
    return "fri3d_badge_2022";
#elifdef CONFIG_FRI3D_BADGE_FOX
    return "fri3d_badge_2024";
#else
    ESP_LOGE(TAG, "unknown board name");
    abort();
#endif
}

void COta::callback_btn_cancel_click(lv_event_t *e)
{
    ESP_LOGI(TAG, "Cancel button clicked");
    COta *thiz = (COta *)lv_event_get_user_data(e);
    thiz->action = Cancel;
}

void COta::callback_btn_check_click(lv_event_t *e)
{
    ESP_LOGI(TAG, "Check button clicked");
    COta *thiz = (COta *)lv_event_get_user_data(e);
    thiz->action = CheckOnline;
}

void COta::callback_drop_down_change(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    uint32_t index = lv_dropdown_get_selected(obj);
    ESP_LOGI(TAG, "Selected version index: %" PRIu32 "", index);
    COta *thiz = (COta *)lv_event_get_user_data(e);
    thiz->selected_version = thiz->available_versions[index];
}

void COta::callback_btn_update_click(lv_event_t *e)
{
    COta *thiz = (COta *)lv_event_get_user_data(e);
    thiz->action = Upgrade;
}

void COta::screen_initial_layout()
{

    lv_lock();

    lv_obj_t *screen = lv_screen_active();

    // Create a container with ROW flex direction
    this->cont_row = lv_obj_create(screen);
    lv_obj_set_width(this->cont_row, lv_obj_get_width(screen));
    lv_obj_set_height(this->cont_row, LV_SIZE_CONTENT);
    lv_obj_align(this->cont_row, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_set_flex_flow(this->cont_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(this->cont_row, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    lv_obj_set_style_pad_hor(this->cont_row, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(this->cont_row, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(this->cont_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(this->cont_row, 0, LV_PART_MAIN);

    lv_obj_t *btn_cancel = lv_button_create(this->cont_row);
    lv_obj_set_size(btn_cancel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_ver(btn_cancel, 5, LV_PART_MAIN);
    lv_obj_t *lbl_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(lbl_cancel, "Cancel");
    lv_obj_center(lbl_cancel);
    lv_obj_add_event_cb(btn_cancel, this->callback_btn_cancel_click, LV_EVENT_CLICKED, this);

    this->btn_check = lv_button_create(this->cont_row);
    lv_obj_set_size(this->btn_check, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_ver(this->btn_check, 5, LV_PART_MAIN);
    lv_obj_t *lbl_check = lv_label_create(this->btn_check);
    lv_label_set_text(lbl_check, "Check Online");
    lv_obj_center(lbl_check);
    lv_obj_add_event_cb(this->btn_check, this->callback_btn_check_click, LV_EVENT_CLICKED, this);

    // Create a container with COLUMN flex direction
    this->cont_col = lv_obj_create(screen);
    lv_obj_set_width(this->cont_col, lv_obj_get_width(screen));
    lv_obj_set_height(this->cont_col, LV_SIZE_CONTENT);
    lv_obj_align(this->cont_col, LV_ALIGN_TOP_MID, 0, 5);

    lv_obj_set_flex_flow(this->cont_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_cross_place(this->cont_col, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_flex_track_place(this->cont_col, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_pad_row(this->cont_col, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(this->cont_col, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(this->cont_col, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(this->cont_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(this->cont_col, 0, LV_PART_MAIN);

    lv_obj_t *lbl_ind_current_version = lv_label_create(this->cont_col);
    lv_label_set_text(lbl_ind_current_version, "Current Version");
    lv_obj_set_size(lbl_ind_current_version, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_t *lbl_current_version = lv_label_create(this->cont_col);
    lv_label_set_text(lbl_current_version, this->current_version);
    lv_obj_set_size(lbl_current_version, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(lbl_current_version, lv_theme_default_get()->color_secondary, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lbl_current_version, LV_OPA_COVER, LV_PART_MAIN);

    lv_unlock();
}

void COta::screen_update_available_versions()
{
    lv_lock();

    lv_obj_t *label_available = lv_label_create(this->cont_col);
    lv_label_set_text(label_available, "Available versions");
    lv_obj_set_size(label_available, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_t *drop_down = lv_dropdown_create(this->cont_col);
    lv_obj_set_width(drop_down, lv_pct(90));
    lv_dropdown_set_options_static(drop_down, this->drop_down_options.c_str());
    lv_obj_add_event_cb(drop_down, this->callback_drop_down_change, LV_EVENT_VALUE_CHANGED, this);
    lv_dropdown_set_selected(drop_down, 0);

    this->selected_version = this->available_versions[0];

    lv_obj_delete(this->btn_check);
    this->btn_check = nullptr;

    lv_obj_t *btn_update = lv_button_create(this->cont_row);
    lv_obj_set_size(btn_update, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_ver(btn_update, 5, LV_PART_MAIN);
    lv_obj_t *lbl_update = lv_label_create(btn_update);
    lv_label_set_text(lbl_update, "Update");
    lv_obj_center(lbl_update);
    lv_obj_add_event_cb(btn_update, this->callback_btn_update_click, LV_EVENT_CLICKED, this);

    lv_unlock();
}

void COta::screen_error_label_show(const char *message)
{
    lv_lock();

    if (this->label_error == nullptr)
    {
        this->label_error = lv_label_create(this->cont_col);
        lv_obj_set_height(this->label_error, LV_SIZE_CONTENT);
        lv_obj_set_style_max_height(this->label_error, 70, LV_PART_MAIN);
        lv_obj_set_width(this->label_error, lv_pct(100));
        lv_label_set_long_mode(this->label_error, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_bg_color(this->label_error, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    }
    lv_label_set_text(this->label_error, message);

    lv_unlock();
}

void COta::screen_error_label_remove()
{
    lv_lock();

    if (this->label_error != nullptr)
    {
        lv_obj_delete(this->label_error);
        this->label_error = nullptr;
    }

    lv_unlock();
}

bool COta::getVisible() const
{
    return true;
}

static COta ota_impl;
Application::CBaseApp &ota = ota_impl;

} // namespace Fri3d::Apps::Ota
