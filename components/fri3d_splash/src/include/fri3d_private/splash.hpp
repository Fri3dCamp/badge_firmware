#pragma once

#include <mutex>
#include <thread>

#include "fri3d_application/app.hpp"

namespace Fri3d::Apps::Splash
{

class CSplash : public Application::CBaseApp
{
private:
    std::thread worker;
    std::mutex workerMutex;
    bool running;
    void work() const;

public:
    CSplash();

    void init() override;
    void deinit() override;

    [[nodiscard]] const char *getName() const override;

    void activate() override;
    void deactivate() override;
};

} // namespace Fri3d::Apps::Splash
