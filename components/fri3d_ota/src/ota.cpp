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
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_private/h2non_semver.h"
#include "fri3d_private/ota.hpp"
#include "fri3d_private/ota_helper.h"
#include "fri3d_private/version_helper.h"
#include "fri3d_private/wifi_connect.h"

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
        else
        {
            ESP_LOGE(TAG, "unkown action: %d", this->action);
        }
    }
}

void COta::deactivate()
{
    ESP_LOGI(TAG, "Deactivated");
    // TODO: free this->available_versions
}

void COta::do_fetch_versions(void)
{
    this->screen_spinner_start("fetching versions...");

    initialize_nvs();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "Connected to AP, begin http example");

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

    const cJSON *tree = cJSON_GetObjectItemCaseSensitive(this->version_task_parameters.json, "tree");
    int tree_len = cJSON_GetArraySize(tree);
    ESP_LOGI(TAG, "tree_len: %d", tree_len);

    this->available_versions = std::vector<ota_version_t>();

    const char *board_name = COta::getBoardName();
    char ota_part[strlen(board_name) + 6];
    strcpy(ota_part, "ota/");
    strcat(ota_part, board_name);
    strcat(ota_part, "/");

    const cJSON *node;
    cJSON_ArrayForEach(node, tree)
    {
        cJSON *type = cJSON_GetObjectItemCaseSensitive(node, "type");
        if (strcmp(type->valuestring, "tree") == 0)
        {
            cJSON *path = cJSON_GetObjectItemCaseSensitive(node, "path");
            char *version;
            int found = COta::path_to_version(path, ota_part, &version);
            if (found > 0)
            {
                ota_version_t v = {version, std::vector<ota_file_t>()};
                this->available_versions.push_back(v);
            }
        }
        else if (strcmp(type->valuestring, "blob") == 0)
        {
            cJSON *path = cJSON_GetObjectItemCaseSensitive(node, "path");
            char *version;
            char *filename;
            int found = COta::path_to_version_filename(path, ota_part, &version, &filename);

            if (found > 0)
            {
                char *url = nullptr;
                cJSON *url_json = cJSON_GetObjectItemCaseSensitive(node, "url");
                if (cJSON_IsString(url_json))
                {
                    url = (char *)calloc(strlen(url_json->valuestring) + 1, sizeof(char));
                    strcpy(url, url_json->valuestring);
                }

                int size = -1;
                cJSON *size_json = cJSON_GetObjectItemCaseSensitive(node, "size");
                if (cJSON_IsNumber(size_json))
                {
                    size = size_json->valueint;
                }

                ota_file_t f = {
                    filename, // name
                    url,      // url
                    size      // size
                };

                // look up the version in the this->available_versions, search in reverse order
                auto it = std::find_if(
                    this->available_versions.rbegin(),
                    this->available_versions.rend(),
                    [version](const ota_version_t a) { return strcmp(a.version, version) == 0; });
                if (it == this->available_versions.rend())
                {
                    ESP_LOGE(TAG, "Could not find in versions, version %s", version);
                }
                else
                {
                    it->files.push_back(f);
                    // ESP_LOGD(TAG, "files.size: %" PRIu16, it->files.size());
                }
            }
        }
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
            int r_a = semver_parse(a.version, &s_a);
            int r_b = semver_parse(b.version, &s_b);
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
        ESP_LOGI(TAG, "this->available_versions[%" PRIu16 "]: %s", i, this->available_versions[i].version);
        this->drop_down_options += this->available_versions[i].version;
        if (i < this->available_versions.size() - 1)
            this->drop_down_options += "\n";
    }
    ESP_LOGD(TAG, "drop_down_options: %s", this->drop_down_options.c_str());

    this->screen_spinner_stop();

    this->screen_update_available_versions();
}

int COta::path_to_version(const cJSON *path, const char *ota_part, char **version)
{
    int found = 0;

    if (path != NULL && cJSON_IsString(path))
    {
        // does path start with ota/{board_name}/
        char *v = strstr(path->valuestring, ota_part);
        if (v != NULL)
        {
            v += strlen(ota_part);
            // no more "/"" in the string
            if (strstr(v, "/") == NULL)
            {
                // v points to char* with the version string
                // ESP_LOGD(TAG, "real version: %s", v);
                *version = (char *)calloc(strlen(v) + 1, sizeof(char));
                if (*version == NULL)
                {
                    ESP_LOGE(TAG, "failed allocating memory for version string");
                }
                else
                {
                    strcpy(*version, v);
                    ESP_LOGD(TAG, "real version: %s, copy_version: %s", v, *version);
                    found = 1;
                }
            }
        }
    }
    return found;
}

int COta::path_to_version_filename(const cJSON *path, const char *ota_part, char **version, char **filename)
{
    int found = 0;

    if (path != NULL && cJSON_IsString(path))
    {
        // ESP_LOGD(TAG, "parsing for filename path: %s", path->valuestring);

        // does path start with ota/{board_name}/
        char *v = strstr(path->valuestring, ota_part);
        if (v != NULL)
        {
            v += strlen(ota_part);
            // next "/"" in the string
            char *f = strstr(v, "/");
            // "/" found an no further
            if (f != NULL && strstr(f + 1, "/") == NULL)
            {
                // v till f points to char* with the version string

                // ESP_LOGD(TAG, "real version: %s", v);
                *version = (char *)calloc(f - v + 1, sizeof(char));
                *filename = (char *)calloc(strlen(f) + 1, sizeof(char));
                if (*version == NULL || *filename == NULL)
                {
                    ESP_LOGE(TAG, "failed allocating memory for version or filename string");
                }
                else
                {
                    memcpy(*version, v, f - v);
                    strcpy(*filename, f + 1);
                    // ESP_LOGD(TAG, "path version: %s, path filename: %s", *version, *filename);
                    found = 1;
                }
            }
        }
    }
    return found;
}

void COta::do_upgrade()
{
    this->screen_spinner_start("Upgrading...");

    initialize_nvs();

    ESP_ERROR_CHECK(esp_netif_init());
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_ERROR_CHECK(err);
    }

    ESP_ERROR_CHECK(esp_event_handler_register(ESP_HTTPS_OTA_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    // wifi should still be connected from fetch versions
    // ESP_ERROR_CHECK(example_connect());

    /* Ensure to disable any WiFi power save mode, this allows best throughput
     * and hence timings for overall OTA operation.
     */
    esp_wifi_set_ps(WIFI_PS_NONE);

    ESP_LOGD(TAG, "this->selected_version.version: %s", this->selected_version.version);
    const char *upgrade_filename = "micropython.bin";
    this->upgrade_task_parameters = {};
    auto it = std::find_if(
        this->selected_version.files.begin(),
        this->selected_version.files.end(),
        [upgrade_filename](const ota_file_t a) { return strcmp(a.name, upgrade_filename) == 0; });
    if (it == this->selected_version.files.end())
    {
        ESP_LOGE(TAG, "Could not find in versions, version %s", upgrade_filename);
    }
    else
    {
        this->upgrade_task_parameters.url = it->url;
        this->upgrade_task_parameters.size = it->size;
    }

    xTaskCreatePinnedToCore(
        &advanced_ota_example_task,
        "advanced_ota_example_task",
        1024 * 8,
        &this->upgrade_task_parameters,
        5,
        NULL,
        0);
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

void COta::screen_spinner_start(const char *label_text)
{
    lv_lock();

    lv_obj_t *screen = lv_screen_active();
    this->cont_spinner = lv_obj_create(screen);
    lv_obj_set_size(this->cont_spinner, lv_obj_get_width(screen), lv_obj_get_height(screen));
    lv_obj_center(this->cont_spinner);
    lv_obj_set_style_bg_opa(this->cont_spinner, LV_OPA_COVER, LV_PART_MAIN);

    int32_t size = std::min<int32_t>(lv_obj_get_width(screen) / 2, lv_obj_get_height(screen) / 2);

    lv_obj_t *spinner = lv_spinner_create(this->cont_spinner);
    lv_obj_set_size(spinner, size, size);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -10);
    lv_spinner_set_anim_params(spinner, 1000, 200);

    this->spinner_label = lv_label_create(this->cont_spinner);
    lv_label_set_text(this->spinner_label, label_text);
    lv_obj_align(this->spinner_label, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_unlock();
}

void COta::screen_spinner_stop()
{
    lv_lock();
    lv_obj_delete(this->cont_spinner);
    lv_unlock();
}

void COta::screen_spinner_add_progress_bar_message(const char *progress_message)
{
    lv_lock();

    lv_obj_t *screen = lv_screen_active();
    this->progress_bar = lv_bar_create(this->cont_spinner);
    lv_obj_set_size(this->progress_bar, lv_obj_get_width(screen) - 20, 20);
    lv_obj_align_to(this->progress_bar, this->spinner_label, LV_ALIGN_OUT_TOP_MID, 0, -5);
    lv_bar_set_value(this->progress_bar, 0, LV_ANIM_OFF);

    lv_obj_t *progress_label = lv_label_create(this->cont_spinner);
    lv_obj_align_to(progress_label, this->progress_bar, LV_ALIGN_OUT_TOP_MID, 0, -5);
    lv_label_set_text(progress_label, progress_message);

    lv_unlock();
}

void COta::screen_spinner_set_progress_bar_value(int32_t value)
{
    lv_lock();
    if (value != lv_bar_get_value(this->progress_bar))
    {
        ESP_LOGD(TAG, "progress bar value: %" PRIi32 "", value);
        lv_bar_set_value(this->progress_bar, value, LV_ANIM_OFF);
    }
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
