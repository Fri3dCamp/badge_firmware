#include <chrono>

#include "esp_log.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_bsp/bsp.h"
#include "fri3d_private/splash.hpp"
#include "fri3d_util/lvgl/animated_logo.h"

#ifdef CONFIG_FRI3D_BUZZER
#include "fri3d_util/rtttl/rtttl.h"
#include "fri3d_util/rtttl/rtttl_songs.h"
#endif

using namespace std::literals;

#define SPLASH_DURATION 8s

namespace Fri3d::Apps::Splash
{

static const char *TAG = "Fri3d::Apps::Splash::CSplash";

CSplash::CSplash()
    : screen(nullptr)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CSplash::init()
{
    ESP_LOGI(TAG, "Initializing splash");
    if (this->screen == nullptr)
    {
        lv_lock();
        this->screen = lv_obj_create(nullptr);
        lv_unlock();
    }
}

void CSplash::deinit()
{
    ESP_LOGI(TAG, "Deinitializing splash");
    if (this->screen != nullptr)
    {
        lv_lock();
        lv_obj_delete(this->screen);
        this->screen = nullptr;
        lv_unlock();
    }
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

    auto logo =
        fri3d_lv_animated_logo_create(this->screen, lv_obj_get_width(this->screen), lv_obj_get_height(this->screen));
    lv_obj_center(logo);

    lv_screen_load(this->screen);

    lv_unlock();

    // We want the splash screen to display for at least 3 seconds, but longer if the song takes longer
    auto start = std::chrono::steady_clock::now();
#ifdef CONFIG_FRI3D_BUZZER
    play_rtttl(dump_dump_s, 20);
#endif
    auto length = std::chrono::steady_clock::now() - start;

    if (length < SPLASH_DURATION)
    {
        std::this_thread::sleep_for(SPLASH_DURATION - length);
    }

    lv_lock();
    lv_obj_clean(this->screen);
    lv_unlock();

    ESP_ERROR_CHECK(led_indicator_set_on_off(leds[0], false));
    ESP_ERROR_CHECK(led_indicator_delete(leds[0]));

    this->getAppManager().previousApp();
}

bool CSplash::getVisible() const
{
    return false;
}

static CSplash splash_impl;
Application::CBaseApp &splash = splash_impl;

} // namespace Fri3d::Apps::Splash
