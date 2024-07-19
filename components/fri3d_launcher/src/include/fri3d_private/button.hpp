#pragma once

#include "lvgl.h"

namespace Fri3d::Apps::Launcher
{

class CButton
{
private:
    lv_obj_t *button;
    lv_obj_t *label;

    uint8_t count{};
    int offset;

    void setText();
    static void click(lv_event_t *event);

public:
    CButton(int offset);
    ~CButton();

    void show();
    void hide();
};

} // namespace Fri3d::Apps::Launcher
