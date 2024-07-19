#include <chrono>

#include "esp_log.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_bsp/bsp.h"
#include "fri3d_private/splash.hpp"
#include "fri3d_util/rtttl/rtttl.h"
#include "fri3d_util/rtttl/rtttl_songs.h"

using namespace std::literals;

namespace Fri3d::Apps::Splash
{

static const char *TAG = "Fri3d::Apps::Splash::CSplash";

CSplash::CSplash()
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CSplash::init()
{
    ESP_LOGI(TAG, "Initializing splash");
}

void CSplash::deinit()
{
    ESP_LOGI(TAG, "Deinitializing splash");
}

const char *CSplash::getName() const
{
    return "Splash";
}

void CSplash::activate()
{
    std::lock_guard lock(this->workerMutex);
    if (this->worker.joinable())
    {
        throw std::runtime_error("Already running");
    }

    this->running = true;
    this->worker = std::thread(&CSplash::work, this);

    ESP_LOGD(TAG, "Activated");
}

void CSplash::deactivate()
{
    std::lock_guard lock(this->workerMutex);
    if (this->worker.joinable())
    {
        this->running = false;
        this->worker.join();
    }

    ESP_LOGD(TAG, "Deactivated");
}

void CSplash::work() const
{
    ESP_LOGI(TAG, "Showing splash screen");

    static led_indicator_handle_t leds[1];
    ESP_ERROR_CHECK(bsp_led_indicator_create(leds, nullptr, 1));
    ESP_ERROR_CHECK(led_indicator_start(leds[0], BSP_LED_BLINK_FLOWING));

    lv_lock();

    auto screen = lv_screen_active();

    auto label = lv_label_create(screen);

    lv_label_set_text(label, "SPLASH SCREEN");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, label);
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

    // We want the splash screen to display for at least 3 seconds, but longer if the song takes longer
    auto start = std::chrono::steady_clock::now();
    play_rtttl(dump_dump_s, 20);
    auto length = std::chrono::steady_clock::now() - start;

    if (length < 3s)
    {
        std::this_thread::sleep_for(3s - length);
    }

    lv_lock();
    lv_anim_delete(&a, reinterpret_cast<lv_anim_exec_xcb_t>(lv_obj_set_y));
    lv_obj_delete(label);
    lv_unlock();

    ESP_ERROR_CHECK(led_indicator_set_on_off(leds[0], false));
    ESP_ERROR_CHECK(led_indicator_delete(leds[0]));

    this->getAppManager().previousApp();
}

static CSplash splash_impl;
Application::CBaseApp &splash = splash_impl;

} // namespace Fri3d::Apps::Splash
