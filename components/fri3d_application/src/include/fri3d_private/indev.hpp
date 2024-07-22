#pragma once

#include "lvgl.h"

#include "fri3d_bsp/bsp.h"

#ifdef BSP_CAPS_BUTTONS
#include "fri3d_private/indev_buttons.hpp"
#endif

#ifdef BSP_CAPS_JOYSTICK
#include "fri3d_private/indev_joystick.hpp"
#endif

namespace Fri3d::Application
{

class CIndev
{
private:
    lv_indev_t *indev;
    lv_group_t *group;

    enum InputType
    {
#ifdef BSP_CAPS_BUTTONS
        Buttons,
#endif
#ifdef BSP_CAPS_JOYSTICK
        Joystick,
#endif
        None
    };

#ifdef BSP_CAPS_BUTTONS
    CIndevButtons buttons;
#endif
#ifdef BSP_CAPS_JOYSTICK
    CIndevJoystick joystick;
#endif

    InputType lastInput;

    static void readInputs(lv_indev_t *indev, lv_indev_data_t *data);

    template <class T> bool readInputs(T &input, InputType inputType, lv_indev_data_t *data)
    {
        bool shouldReturn = false;

        if (this->lastInput == InputType::None || this->lastInput == inputType)
        {
            if (input.readInputs(data))
            {
                this->lastInput = inputType;
                shouldReturn = true;
            }
            else if (this->lastInput == inputType)
            {
                this->lastInput = InputType::None;
                shouldReturn = true;
            }
        }

        return shouldReturn;
    }

public:
    CIndev();

    void init();
    void deinit();
};

} // namespace Fri3d::Application
