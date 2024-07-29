#pragma once

#include "fri3d_application/app.hpp"
#include "fri3d_application/hardware_manager.hpp"

namespace Fri3d::Application
{

class CBaseApp::impl
{
private:
    IAppManager *appManager;
    IHardwareManager *hardwareManager;

public:
    void setAppManager(IAppManager *value);
    [[nodiscard]] IAppManager &getAppManager() const;

    void setHardwareManager(IHardwareManager *value);
    [[nodiscard]] IHardwareManager &getHardwareManager() const;
};

} // namespace Fri3d::Application
