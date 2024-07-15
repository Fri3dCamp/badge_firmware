#include <chrono>
#include <thread>

#include "esp_log.h"

#include "fri3d_private/application.hpp"

using namespace std::chrono_literals;

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CApplication";

CApplication::CApplication() :
    running(false),
    initialized(false)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CApplication::init()
{
    ESP_LOGI(TAG, "Initializing application");
    this->appManager.init();
}

void CApplication::deinit()
{
    this->appManager.deinit();
    ESP_LOGI(TAG, "Deinitializing application");
}

IAppManager &CApplication::getAppManager()
{
    return this->appManager;
}

void CApplication::run(const CBaseApp &app)
{
    this->appManager.setDefaultApp(app);

    this->appManager.start();

    this->appManager.activateDefaultApp();

    ESP_LOGI(TAG, "Starting application loop");
    this->running = true;

    while (this->running)
    {
        std::this_thread::sleep_for(5000ms);
    }

    this->appManager.stop();
}

static CApplication application_impl;
IApplication &application = application_impl;

}
