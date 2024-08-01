#include "esp_log.h"

#include "esp_wifi.h"

#include "fri3d_application/lvgl/wait_dialog.hpp"
#include "fri3d_private/hardware_wifi.hpp"

namespace Fri3d::Application::Hardware
{

static const char *TAG = "Fri3d::Application::Hardware::CWifi";

CWifi::CWifi()
    : networkInterface(nullptr)
    , instanceAnyWifi(nullptr)
    , instanceGotIP(nullptr)
    , wifiConfig({})
    , connected(false)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));

    memcpy(wifiConfig.sta.ssid, CONFIG_FRI3D_DEFAULT_WIFI_SSID, strlen(CONFIG_FRI3D_DEFAULT_WIFI_SSID));
    memcpy(wifiConfig.sta.password, CONFIG_FRI3D_DEFAULT_WIFI_PASSWORD, strlen(CONFIG_FRI3D_DEFAULT_WIFI_PASSWORD));
    wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_OPEN;
    wifiConfig.sta.threshold.rssi = -127;
}

void CWifi::connect()
{
    ESP_LOGI(TAG, "Enabling wifi");

    // Start the network interface
    ESP_ERROR_CHECK(esp_netif_init());
    this->networkInterface = esp_netif_create_default_wifi_sta();

    // Start the wifi driver
    wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfig));

    // Register the event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        CWifi::eventHandler,
        this,
        &this->instanceAnyWifi));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        CWifi::eventHandler,
        this,
        &this->instanceAnyWifi));

    // Configure the wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &this->wifiConfig));

    // Start the wifi
    ESP_ERROR_CHECK(esp_wifi_start());
}

void CWifi::disconnect()
{
    {
        std::lock_guard<std::mutex> lock(this->connectedMutex);
        this->connected = false;
    }

    ESP_LOGI(TAG, "Disabling wifi");
    // Stop the wifi
    ESP_ERROR_CHECK(esp_wifi_stop());

    // Remove event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, this->instanceGotIP));
    this->instanceGotIP = nullptr;

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, this->instanceAnyWifi));
    this->instanceAnyWifi = nullptr;

    // Stop the wifi driver
    ESP_ERROR_CHECK(esp_wifi_deinit());

    // Stop the network interface
    esp_netif_destroy_default_wifi(this->networkInterface);
    this->networkInterface = nullptr;
    ESP_ERROR_CHECK(esp_netif_deinit());
}

void CWifi::init()
{
    ESP_LOGI(TAG, "Initializing");
}

void CWifi::deinit()
{
    this->disconnect();
    ESP_LOGI(TAG, "Deinitializing");
}

bool CWifi::getConnected()
{
    std::lock_guard<std::mutex> lock(this->connectedMutex);
    return this->connected;
}

void CWifi::eventHandler(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData)
{
    auto self = static_cast<CWifi *>(arg);

    if (eventBase == WIFI_EVENT)
    {
        switch (eventID)
        {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "Connecting to `%s`", self->wifiConfig.sta.ssid);
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "Disconnected, trying to reconnect to `%s`", self->wifiConfig.sta.ssid);
            esp_wifi_connect();
            break;
        default:
            break;
        }
    }
    else if (eventBase == IP_EVENT)
    {
        if (eventID == IP_EVENT_STA_GOT_IP)
        {
            auto *data = static_cast<ip_event_got_ip_t *>(eventData);
            ESP_LOGI(TAG, "Configured ip:" IPSTR, IP2STR(&data->ip_info.ip));

            {
                std::lock_guard<std::mutex> lock(self->connectedMutex);
                self->connected = true;
            }
            self->connectedSignal.notify_all();
        }
    }
}

bool CWifi::waitOnConnect(std::chrono::seconds timeout, bool showDialog)
{
    // We create a copy of the current connected state with a proper lock, so we know beforehand whether we need to
    // create the WaitDialog or not.
    bool result = this->getConnected();

    if (!result)
    {
        std::unique_lock<std::mutex> lock(this->connectedMutex);
        LVGL::CWaitDialog dialog("Waiting for wifi to connect.");

        if (showDialog)
        {
            dialog.show();
        }

        connectedSignal.wait_for(lock, timeout, [this] { return this->connected; });

        if (showDialog)
        {
            dialog.hide();
        }

        result = this->connected;
    }

    return result;
}

} // namespace Fri3d::Application::Hardware
