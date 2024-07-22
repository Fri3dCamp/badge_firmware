#include "esp_log.h"

#include "fri3d_application/lvgl.hpp"

#include "fri3d_private/indev.hpp"

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CIndev";

CIndev::CIndev()
    : indev(nullptr)
    , group(nullptr)
#if BSP_CAPS_BUTTONS
    , buttons()
#endif
#if BSP_CAPS_JOYSTICK
    , joystick()
#endif
    , lastInput(None)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CIndev::init()
{
    ESP_LOGI(TAG, "Initializing");

    // We need to initialize the inputs before LVGL because as soon as Indev is initialized readInputs() will start
    // reading values from them.
#if BSP_CAPS_BUTTONS
    this->buttons.init();
#endif
#if BSP_CAPS_JOYSTICK
    this->joystick.init();
#endif

    lv_lock();

    this->indev = lv_indev_create();
    lv_indev_set_type(this->indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(this->indev, CIndev::readInputs);
    lv_indev_set_user_data(this->indev, this);
    lv_indev_set_display(this->indev, lv_display_get_default());

    this->group = lv_group_create();
    lv_group_set_default(this->group);
    lv_indev_set_group(this->indev, this->group);
    lv_indev_enable(this->indev, true);

    lv_unlock();
}

void CIndev::deinit()
{
    ESP_LOGI(TAG, "Deinitializing");

    lv_lock();

    lv_indev_enable(this->indev, false);
    lv_indev_delete(this->indev);
    lv_group_delete(this->group);

    lv_unlock();

#if BSP_CAPS_JOYSTICK
    joystick.deinit();
#endif
#if BSP_CAPS_BUTTONS
    buttons.deinit();
#endif
}

void CIndev::readInputs(lv_indev_t *indev, lv_indev_data_t *data)
{
    auto self = static_cast<CIndev *>(lv_indev_get_user_data(indev));

    // We only allow one button at a time
    data->continue_reading = false;

#if BSP_CAPS_JOYSTICK
    if (self->readInputs(self->joystick, InputType::Joystick, data))
    {
        return;
    }
#endif

#if BSP_CAPS_BUTTONS
    if (self->readInputs(self->buttons, InputType::Buttons, data))
    {
        return;
    }
#endif
}

} // namespace Fri3d::Application
