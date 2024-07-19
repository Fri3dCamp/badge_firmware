#pragma once

#include <map>
#include <mutex>
#include <queue>

#include "lvgl.h"

#include "fri3d_bsp/bsp.h"

namespace Fri3d::Application
{

class CIndev
{
private:
    lv_indev_t *indev;
    lv_group_t *group;

    // Note: this function is quite heavy, only use it for initializing a lookup table
    static _lv_key_t keymap(bsp_button_t button);

    typedef std::map<button_handle_t, _lv_key_t> CKeymap;
    button_handle_t buttons[BSP_BUTTON_NUM];
    CKeymap mapping;

    enum EventType
    {
        ButtonPressed,
        ButtonReleased,
    };

    typedef std::pair<EventType, button_handle_t> CEvent;
    typedef std::queue<CEvent> CEvents;
    CEvents events;
    std::mutex eventsMutex;

    void buttonEvent(EventType eventType, button_handle_t button);

    static void buttonPressed(void *button, void *data);
    static void buttonReleased(void *button, void *data);
    static void readButtons(lv_indev_t *indev, lv_indev_data_t *data);

public:
    CIndev();

    void init();
    void deinit();
};

} // namespace Fri3d::Application
