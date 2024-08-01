#pragma once

#include <chrono>

using namespace std::literals;

namespace Fri3d::Application::Hardware
{

class IWifi
{
public:
    /**
     * @brief connect the wifi
     *
     * The connection runs in a background task and this function will return immediately, use connected() to know when
     * the connection is ready
     */
    virtual void connect() = 0;

    /**
     * @brief disconnect the wifi
     */
    virtual void disconnect() = 0;

    /**
     * @brief check if the wifi is connected
     *
     * @return bool to indicate the state
     */
    [[nodiscard]] virtual bool getConnected() = 0;

    /**
     * @brief wait until the wifi is connected or the timeout has occurred
     *
     * @param timeout timeout
     * @return bool to indicate if connected
     */
    virtual bool waitOnConnect(std::chrono::seconds timeout, bool showDialog) = 0;
};

} // namespace Fri3d::Application::Hardware
