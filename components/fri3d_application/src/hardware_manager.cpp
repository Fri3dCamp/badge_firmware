#include "esp_event.h"
#include "esp_log.h"

#include "fri3d_private/hardware_manager.hpp"

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CHardwareManager";

CHardwareManager::CHardwareManager()
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CHardwareManager::init()
{
    // Some hardware drivers depend on an event loop, so we make sure it is running here
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize drivers
    this->wifi.init();

    ESP_LOGI(TAG, "Initialized");
}

void CHardwareManager::deinit()
{
    // Deinitialize drivers
    this->wifi.deinit();

    // Disable the default event loop
    ESP_ERROR_CHECK(esp_event_loop_delete_default());

    ESP_LOGI(TAG, "Deinitialized");
}

Hardware::IWifi &CHardwareManager::getWifi()
{
    return this->wifi;
}

} // namespace Fri3d::Application
