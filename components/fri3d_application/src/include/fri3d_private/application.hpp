#pragma once

#include "fri3d_application/application.hpp"
#include "fri3d_private/app_manager.hpp"

namespace Fri3d::Application
{

class CApplication : public IApplication
{
private:
    CAppManager appManager;
    bool running;
    bool initialized;

public:
    CApplication();

    void init() override;
    void deinit() override;

    IAppManager &getAppManager() override;

    void run(const CBaseApp &app) override;
};

} // namespace Fri3d::Application
