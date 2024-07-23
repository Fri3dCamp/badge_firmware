#include "esp_log.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_private/hello.hpp"

namespace Fri3d::Apps::Hello
{

static const char *TAG = "Fri3d::Apps::Hello::CHello";

CHello::CHello()
    : screen(nullptr)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CHello::init()
{
    ESP_LOGI(TAG, "Initializing hello");
    if (this->screen == nullptr)
    {
        lv_lock();
        this->screen = lv_obj_create(nullptr);
        lv_unlock();
    }
}

void CHello::deinit()
{
    ESP_LOGI(TAG, "Deinitializing hello");
    if (this->screen != nullptr)
    {
        lv_lock();
        lv_obj_delete(this->screen);
        this->screen = nullptr;
        lv_unlock();
    }
}

const char *CHello::getName() const
{
    return "Hello";
}

void CHello::activate()
{
    lv_lock();

    auto button = lv_button_create(this->screen);
    lv_obj_align(button, LV_ALIGN_CENTER, 0, -20);
    lv_obj_add_event_cb(button, CHello::clickEvent, LV_EVENT_CLICKED, this);

    auto button_label = lv_label_create(button);
    lv_label_set_text(button_label, "HELLO WORLD!");

    auto label = lv_label_create(this->screen);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60);
    lv_label_set_text(label, "Press A to exit to main menu");

    lv_screen_load(this->screen);

    lv_unlock();

    ESP_LOGD(TAG, "Activated");
}

void CHello::deactivate()
{
    lv_lock();
    lv_obj_clean(this->screen);
    lv_unlock();

    ESP_LOGD(TAG, "Deactivated");
}

void CHello::clickEvent(lv_event_t *event)
{
    ESP_LOGI(TAG, "Return pressed, deactivating");
    auto self = static_cast<CHello *>(lv_event_get_user_data(event));

    self->getAppManager().previousApp();
}

bool CHello::getVisible() const
{
    return true;
}

static CHello hello_impl;
Application::CBaseApp &hello = hello_impl;

} // namespace Fri3d::Apps::Hello
