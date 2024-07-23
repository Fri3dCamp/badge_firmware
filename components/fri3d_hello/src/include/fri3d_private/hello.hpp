#pragma once

#include <mutex>
#include <thread>

#include "lvgl.h"

#include "fri3d_application/app.hpp"

namespace Fri3d::Apps::Hello
{

class CHello : public Application::CBaseApp
{
private:
    lv_obj_t *screen;

    static void clickEvent(lv_event_t *event);

public:
    CHello();

    void init() override;
    void deinit() override;

    [[nodiscard]] const char *getName() const override;
    [[nodiscard]] bool getVisible() const override;

    void activate() override;
    void deactivate() override;
};

} // namespace Fri3d::Apps::Hello
