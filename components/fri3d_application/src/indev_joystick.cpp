#include "esp_log.h"

#include "fri3d_application/lvgl.hpp"

#include "fri3d_private/indev_joystick.hpp"

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CIndevJoystick";

CIndevJoystick::CIndevJoystick()
    : adc()
    , joystick()
    , joystickLast(UINT32_MAX)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CIndevJoystick::init()
{
    ESP_LOGI(TAG, "Initializing");

    ESP_ERROR_CHECK(bsp_adc_create(&this->adc));
    ESP_ERROR_CHECK(bsp_joystick_create(this->adc, this->joystick, nullptr, BSP_JOYSTICK_AXIS_NUM));
}

void CIndevJoystick::deinit()
{
    ESP_LOGI(TAG, "Deinitializing");

    for (auto axis : this->joystick)
    {
        ESP_ERROR_CHECK(joystick_axis_delete(axis));
    }

    ESP_ERROR_CHECK(adc_driver_delete(this->adc));
}

bool CIndevJoystick::readInputs(lv_indev_data_t *data)
{
    bool keepControl = false;

    int8_t x, y;
    uint32_t joystickNext = UINT32_MAX;

    joystick_axis_read(this->joystick[BSP_ADC_CHANNEL_JOYSTICK_AXIS_X], &x);
    joystick_axis_read(this->joystick[BSP_ADC_CHANNEL_JOYSTICK_AXIS_Y], &y);

    if (x < -90)
    {
        joystickNext = LV_KEY_LEFT;
    }
    else if (x > 90)
    {
        joystickNext = LV_KEY_RIGHT;
    }
    else if (y < -90)
    {
        joystickNext = LV_KEY_DOWN;
    }
    else if (y > 90)
    {
        joystickNext = LV_KEY_UP;
    }

    if (this->joystickLast == UINT32_MAX)
    {
        // No button was pressed last time and a button is pressed now
        if (joystickNext != UINT32_MAX)
        {
            this->joystickLast = joystickNext;

            data->key = joystickNext;
            data->state = LV_INDEV_STATE_PRESSED;

            keepControl = true;
            ESP_LOGV(TAG, "readButtons: (button: %" PRIu32 ", PRESSED)", data->key);
        }
    }
    else
    {
        if (joystickNext == this->joystickLast)
        {
            // The button is still pressed
            data->key = joystickNext;
            data->state = LV_INDEV_STATE_PRESSED;

            keepControl = true;
            ESP_LOGV(TAG, "readButtons: (button: %" PRIu32 ", PRESSED)", data->key);
        }
        else
        {
            // The button was released and possibly a new button has already been pressed
            this->joystickLast = joystickNext;

            data->key = joystickNext;
            data->state = LV_INDEV_STATE_RELEASED;

            keepControl = true;
            ESP_LOGV(TAG, "readButtons: (button: %" PRIu32 ", PRESSED)", data->key);
        }
    }

    return keepControl;
}

} // namespace Fri3d::Application
