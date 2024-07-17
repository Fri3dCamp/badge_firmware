#include "esp_log.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_private/launcher.hpp"

namespace Fri3d::Apps::Launcher
{

static const char *TAG = "Fri3d::Apps::Launcher::CLauncher";

CLauncher::CLauncher()
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CLauncher::init()
{
    ESP_LOGI(TAG, "Initializing launcher");
}

void CLauncher::deinit()
{
    ESP_LOGI(TAG, "Deinitializing launcher");
}

const char *CLauncher::getName() const
{
    return "Launcher";
}

void CLauncher::activate()
{
    ESP_LOGD(TAG, "Activated");

    ESP_LOGD(TAG, "Fetching registered apps");
    const auto &apps = this->getAppManager().getApps();

    ESP_LOGD(TAG, "Found %d apps", apps.size());

    for (auto app : apps)
    {
        ESP_LOGV(TAG, "Found %s", app->getName());
    }

    lv_lock();

    auto screen = lv_screen_active();

    this->label = lv_label_create(screen);
    lv_label_set_text(this->label, "SPLASH SCREEN");
    lv_obj_set_style_text_color(this->label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(this->label, LV_ALIGN_CENTER, 0, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, this->label);
    lv_anim_set_values(&a, 10, 50);
    lv_anim_set_duration(&a, 1000);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_playback_duration(&a, 300);
    lv_anim_set_repeat_delay(&a, 500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, reinterpret_cast<lv_anim_exec_xcb_t>(lv_obj_set_y));
    lv_anim_start(&a);

    lv_unlock();
}

void CLauncher::deactivate()
{
    ESP_LOGD(TAG, "Deactivated");
}

static CLauncher launcher_impl;
Application::CBaseApp &launcher = launcher_impl;

} // namespace Fri3d::Apps::Launcher
