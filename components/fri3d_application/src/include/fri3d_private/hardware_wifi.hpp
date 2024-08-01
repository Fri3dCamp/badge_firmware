#pragma once

#include <condition_variable>

#include "esp_wifi.h"

#include "fri3d_application/hardware_wifi.hpp"

namespace Fri3d::Application::Hardware
{

class CWifi : public IWifi
{
private:
    esp_netif_t *networkInterface;
    esp_event_handler_instance_t instanceAnyWifi;
    esp_event_handler_instance_t instanceGotIP;

    wifi_config_t wifiConfig;

    bool connected;
    std::mutex connectedMutex;
    std::condition_variable connectedSignal;

    static void eventHandler(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData);

public:
    CWifi();

    void init();
    void deinit();

    void connect() override;
    void disconnect() override;

    [[nodiscard]] bool getConnected() override;
    bool waitOnConnect(std::chrono::seconds timeout, bool showDialog) override;
};

} // namespace Fri3d::Application::Hardware
