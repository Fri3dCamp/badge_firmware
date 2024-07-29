#pragma once

#include "fri3d_application/hardware_wifi.hpp"

namespace Fri3d::Application
{

class IHardwareManager
{
public:
    virtual Hardware::IWifi &getWifi() = 0;
};

} // namespace Fri3d::Application
