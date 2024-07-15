#include <fri3d_application/app.hpp>

#include "esp_log.h"

#include "fri3d_private/app.hpp"

namespace Fri3d::Application
{

static const char *TAG = "CBaseApp";

CBaseApp::CBaseApp() : base{ std::make_unique<CBaseApp::impl>() }
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

IAppManager &CBaseApp::getAppManager() const
{
    return this->base->getAppManager();
}

}
