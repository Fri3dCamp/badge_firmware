#include "esp_log.h"

#include "fri3d_application/lvgl.hpp"

#include "fri3d_private/indev.hpp"

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CIndev";

CIndev::CIndev()
    : indev(nullptr)
    , group(nullptr)
    , buttons()
    , pressedButton(nullptr)
    , pressedLast(nullptr)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CIndev::init()
{
    ESP_LOGI(TAG, "Initializing");

    lv_lock();

    this->indev = lv_indev_create();
    lv_indev_set_type(this->indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(this->indev, CIndev::readButtons);
    lv_indev_set_user_data(this->indev, this);
    lv_indev_set_display(this->indev, lv_display_get_default());

    this->group = lv_group_create();
    lv_group_set_default(this->group);
    lv_indev_set_group(this->indev, this->group);
    lv_indev_enable(this->indev, true);

    lv_unlock();

    ESP_ERROR_CHECK(bsp_iot_button_create(this->buttons, nullptr, BSP_BUTTON_NUM));
    for (int i = 0; i < BSP_BUTTON_NUM; i++)
    {
        auto button = this->buttons[i];

        this->mapping[button] = CIndev::keymap(static_cast<bsp_button_t>(i));

        ESP_ERROR_CHECK(iot_button_register_cb(button, BUTTON_PRESS_DOWN, CIndev::buttonPressed, this));
        ESP_ERROR_CHECK(iot_button_register_cb(button, BUTTON_PRESS_UP, CIndev::buttonReleased, this));
    }
}

void CIndev::deinit()
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

    lv_lock();

    lv_indev_enable(this->indev, false);
    lv_indev_delete(this->indev);
    lv_group_delete(this->group);

    lv_unlock();
}

void CIndev::readButtons(lv_indev_t *indev, lv_indev_data_t *data)
{
    auto self = static_cast<CIndev *>(lv_indev_get_user_data(indev));

    // We only allow one button at a time
    data->continue_reading = false;

    std::lock_guard lock(self->pressMutex);

    if (self->pressedLast == nullptr)
    {
        // No button was pressed last iteration and a button is pressed now
        if (self->pressedButton != nullptr)
        {
            self->pressedLast = self->pressedButton;

            data->key = self->mapping[self->pressedLast];
            data->state = LV_INDEV_STATE_PRESSED;
            ESP_LOGV(TAG, "readButtons: (button: %" PRIu32 ", PRESSED)", data->key);
        }
    }
    else
    {
        if (self->pressedButton == self->pressedLast)
        {
            // The button is still pressed
            data->key = self->mapping[self->pressedLast];
            data->state = LV_INDEV_STATE_PRESSED;
            ESP_LOGV(TAG, "readButtons: (button: %" PRIu32 ", PRESSED)", data->key);
        }
        else
        {
            // The button was released and possibly a new button has already been pressed
            self->pressedLast = self->pressedButton;

            data->key = self->mapping[self->pressedLast];
            data->state = LV_INDEV_STATE_RELEASED;
            ESP_LOGV(TAG, "readButtons: (button: %" PRIu32 ", RELEASED)", data->key);
        }
    }
}

void CIndev::buttonPressed(void *button, void *data)
{
    auto self = static_cast<CIndev *>(data);
    std::lock_guard lock(self->pressMutex);

    if (self->pressedButton == nullptr)
    {
        self->pressedButton = button;
    }
}

void CIndev::buttonReleased(void *button, void *data)
{
    auto self = static_cast<CIndev *>(data);
    std::lock_guard lock(self->pressMutex);

    if (self->pressedButton == button)
    {
        self->pressedButton = nullptr;
    }
}

_lv_key_t CIndev::keymap(bsp_button_t button)
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
