#pragma once

#include "fri3d_application/application.hpp"
#include "fri3d_private/app_manager.hpp"
#include "fri3d_private/lvgl.hpp"

namespace Fri3d::Application
{

class CApplication : public IApplication
{
private:
    bool running;
    bool initialized;

    CAppManager appManager;
    CLVGL lvgl;

public:
    CApplication();

    void init() override;

    void deinit() override;

    IAppManager &getAppManager() override;

    void run(const CBaseApp &app) override;
};

} // namespace Fri3d::Application
