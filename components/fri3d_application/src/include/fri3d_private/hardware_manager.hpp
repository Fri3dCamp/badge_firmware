#pragma once

#include "fri3d_application/hardware_manager.hpp"
#include "fri3d_private/hardware_wifi.hpp"

namespace Fri3d::Application
{

class CHardwareManager : public IHardwareManager
{
private:
    Hardware::CWifi wifi;

public:
    CHardwareManager();

    void init();
    void deinit();

    Hardware::IWifi &getWifi() override;
};

} // namespace Fri3d::Application
