#pragma once

#include "lvgl.h"

#include "fri3d_bsp/bsp.h"

namespace Fri3d::Application
{

// We actually only care about 1 value at the same time so this implementation is simplified a lot
class CIndevJoystick
{
private:
    adc_driver_handle_t adc;

    joystick_axis_handle_t joystick[BSP_JOYSTICK_AXIS_NUM];
    uint32_t joystickLast;

public:
    CIndevJoystick();

    bool readInputs(lv_indev_data_t *data);

    void init();
    void deinit();
};

} // namespace Fri3d::Application
