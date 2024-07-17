#pragma once

#include "fri3d_application/app.hpp"
#include "lvgl.h"

namespace Fri3d::Apps::Launcher
{

class CLauncher : public Application::CBaseApp
{
private:
    lv_obj_t *label;

public:
    CLauncher();

    void init() override;
    void deinit() override;

    [[nodiscard]] const char *getName() const override;

    void activate() override;
    void deactivate() override;
};

} // namespace Fri3d::Apps::Launcher
