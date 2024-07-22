#include "esp_log.h"

#include "fri3d_application/lvgl.hpp"

#include "fri3d_private/indev_buttons.hpp"

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CIndevButtons";

CIndevButtons::CIndevButtons()
    : buttons()
    , pressedButton(nullptr)
    , pressedLast(nullptr)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CIndevButtons::init()
{
    ESP_LOGI(TAG, "Initializing");

    ESP_ERROR_CHECK(bsp_iot_button_create(this->buttons, nullptr, BSP_BUTTON_NUM));
    for (int i = 0; i < BSP_BUTTON_NUM; i++)
    {
        auto button = this->buttons[i];

        this->mapping[button] = CIndevButtons::keymap(static_cast<bsp_button_t>(i));

        ESP_ERROR_CHECK(iot_button_register_cb(button, BUTTON_PRESS_DOWN, CIndevButtons::buttonPressed, this));
        ESP_ERROR_CHECK(iot_button_register_cb(button, BUTTON_PRESS_UP, CIndevButtons::buttonReleased, this));
    }
}

void CIndevButtons::deinit()
{
    ESP_LOGI(TAG, "Deinitializing");

    for (auto button : this->buttons)
    {
        ESP_ERROR_CHECK(iot_button_unregister_cb(button, BUTTON_PRESS_DOWN));
        ESP_ERROR_CHECK(iot_button_unregister_cb(button, BUTTON_PRESS_UP));
    }

    {
        // Clear the buttons
        std::lock_guard lock(this->pressMutex);
        pressedButton = nullptr;
        pressedLast = nullptr;
    }

    for (auto button : this->buttons)
    {
        ESP_ERROR_CHECK(iot_button_delete(button));
    }

    this->mapping.clear();
}

bool CIndevButtons::readInputs(lv_indev_data_t *data)
{
    bool keepControl = false;

    std::lock_guard lock(this->pressMutex);

    if (this->pressedLast == nullptr)
    {
        // No button was pressed last iteration and a button is pressed now
        if (this->pressedButton != nullptr)
        {
            this->pressedLast = this->pressedButton;

            data->key = this->mapping[this->pressedLast];
            data->state = LV_INDEV_STATE_PRESSED;
            keepControl = true;
            ESP_LOGV(TAG, "readButtons: (button: %" PRIu32 ", PRESSED)", data->key);
        }
    }
    else
    {
        if (this->pressedButton == this->pressedLast)
        {
            // The button is still pressed
            data->key = this->mapping[this->pressedLast];
            data->state = LV_INDEV_STATE_PRESSED;
            keepControl = true;
            ESP_LOGV(TAG, "readButtons: (button: %" PRIu32 ", PRESSED)", data->key);
        }
        else
        {
            // The button was released and possibly a new button has already been pressed
            this->pressedLast = this->pressedButton;

            data->key = this->mapping[this->pressedLast];
            data->state = LV_INDEV_STATE_RELEASED;
            keepControl = true;
            ESP_LOGV(TAG, "readButtons: (button: %" PRIu32 ", RELEASED)", data->key);
        }
    }

    return keepControl;
}

void CIndevButtons::buttonPressed(void *button, void *data)
{
    auto self = static_cast<CIndevButtons *>(data);
    std::lock_guard lock(self->pressMutex);

    if (self->pressedButton == nullptr)
    {
        self->pressedButton = button;
    }
}

void CIndevButtons::buttonReleased(void *button, void *data)
{
    auto self = static_cast<CIndevButtons *>(data);
    std::lock_guard lock(self->pressMutex);

    if (self->pressedButton == button)
    {
        self->pressedButton = nullptr;
    }
}

_lv_key_t CIndevButtons::keymap(bsp_button_t button)
{
    _lv_key_t result = LV_KEY_HOME;

    switch (button)
    {
#ifdef BSP_KEY_ENTER
    case BSP_KEY_ENTER:
        result = LV_KEY_ENTER;
        break;
#endif
#ifdef BSP_KEY_ESC
    case BSP_KEY_ESC:
        result = LV_KEY_ESC;
        break;
#endif
#ifdef BSP_KEY_NEXT
    case BSP_KEY_NEXT:
        result = LV_KEY_NEXT;
        break;
#endif
#ifdef BSP_KEY_PREV
    case BSP_KEY_PREV:
        result = LV_KEY_PREV;
        break;
#endif
#ifdef BSP_KEY_HOME
    case BSP_KEY_HOME:
        result = LV_KEY_HOME;
        break;
#endif
#ifdef BSP_KEY_END
    case BSP_KEY_END:
        result = LV_KEY_END;
        break;
#endif
#ifdef BSP_KEY_UP
    case BSP_KEY_UP:
        result = LV_KEY_UP;
        break;
#endif
#ifdef BSP_KEY_LEFT
    case BSP_KEY_LEFT:
        result = LV_KEY_LEFT;
        break;
#endif
#ifdef BSP_KEY_DOWN
    case BSP_KEY_DOWN:
        result = LV_KEY_DOWN;
        break;
#endif
#ifdef BSP_KEY_RIGHT
    case BSP_KEY_RIGHT:
        result = LV_KEY_RIGHT;
        break;
#endif
    default:
        break;
    }

    return result;
}

} // namespace Fri3d::Application
