#include "esp_log.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_private/launcher.hpp"

namespace Fri3d::Apps::Launcher
{

static const char *TAG = "Fri3d::Apps::Launcher::CLauncher";

CLauncher::CLauncher()
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CLauncher::init()
{
    ESP_LOGI(TAG, "Initializing launcher");
}

void CLauncher::deinit()
{
    ESP_LOGI(TAG, "Deinitializing launcher");
}

const char *CLauncher::getName() const
{
    return "Launcher";
}

void CLauncher::activate()
{
    ESP_LOGD(TAG, "Activated");

    ESP_LOGD(TAG, "Fetching registered apps");
    const auto &apps = this->getAppManager().getApps();

    ESP_LOGD(TAG, "Found %d apps", apps.size());

    for (auto app : apps)
    {
        ESP_LOGV(TAG, "Found %s", app->getName());
    }
}

void CLauncher::deactivate()
{
    ESP_LOGD(TAG, "Deactivated");
}

static CLauncher launcher_impl;
Application::CBaseApp &launcher = launcher_impl;

} // namespace Fri3d::Apps::Launcher
