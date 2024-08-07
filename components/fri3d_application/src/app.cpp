#include <fri3d_application/app.hpp>

#include "esp_log.h"

#include "fri3d_private/app.hpp"

namespace Fri3d::Application
{

static const char *TAG = "CBaseApp";

CBaseApp::CBaseApp()
    : base{std::make_unique<CBaseApp::impl>()}
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
};

CBaseApp::~CBaseApp() = default;

void CBaseApp::impl::setAppManager(IAppManager *value)
{
    this->appManager = value;
}

IAppManager &CBaseApp::impl::getAppManager() const
{
    return *this->appManager;
}

void CBaseApp::impl::setHardwareManager(IHardwareManager *value)
{
    this->hardwareManager = value;
}

IHardwareManager &CBaseApp::impl::getHardwareManager() const
{
    return *this->hardwareManager;
}

void CBaseApp::impl::setNvsManager(INvsManager *value)
{
    this->nvsManager = value;
}

INvsManager &CBaseApp::impl::getNvsManager() const
{
    return *this->nvsManager;
}

IAppManager &CBaseApp::getAppManager() const
{
    return this->base->getAppManager();
}

IHardwareManager &CBaseApp::getHardwareManager() const
{
    return this->base->getHardwareManager();
}

INvsManager &CBaseApp::getNvsManager() const
{
    return this->base->getNvsManager();
}

} // namespace Fri3d::Application
