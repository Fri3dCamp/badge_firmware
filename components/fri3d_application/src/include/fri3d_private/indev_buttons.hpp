#pragma once

#include <map>
#include <mutex>

#include "lvgl.h"

#include "fri3d_bsp/bsp.h"

namespace Fri3d::Application
{

class CIndevButtons
{
private:
    // Note: this function is quite heavy, only use it for initializing a lookup table
    static _lv_key_t keymap(bsp_button_t button);

    typedef std::map<button_handle_t, _lv_key_t> CKeymap;
    button_handle_t buttons[BSP_BUTTON_NUM];
    CKeymap mapping;

    button_handle_t pressedButton;
    button_handle_t pressedLast;
    std::mutex pressMutex;

    static void buttonPressed(void *button, void *data);
    static void buttonReleased(void *button, void *data);

public:
    CIndevButtons();

    bool readInputs(lv_indev_data_t *data);

    void init();
    void deinit();
};

} // namespace Fri3d::Application
