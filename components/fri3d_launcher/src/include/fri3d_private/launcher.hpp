#pragma once

#include "fri3d_application/app.hpp"
#include "fri3d_private/button.hpp"

namespace Fri3d::Apps::Launcher
{

class CLauncher : public Application::CBaseApp
{
private:
    CButton button1;
    CButton button2;

    bool splashShown;

    void show();
    void hide();

public:
    CLauncher();

    void init() override;
    void deinit() override;

    [[nodiscard]] const char *getName() const override;

    void activate() override;
    void deactivate() override;
};

} // namespace Fri3d::Apps::Launcher
