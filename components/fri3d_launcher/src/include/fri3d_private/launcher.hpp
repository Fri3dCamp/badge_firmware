#pragma once

#include "fri3d_application/app.hpp"

namespace Fri3d::Apps::Launcher
{

class CLauncher : public Application::CBaseApp
{
public:
    CLauncher();

    void init() override;
    void deinit() override;

    [[nodiscard]] const char *getName() const override;

    void activate() override;
    void deactivate() override;
};

} // namespace Fri3d::Apps::Launcher
