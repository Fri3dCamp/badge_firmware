#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "fri3d_application/app_manager.hpp"

namespace Fri3d::Application
{

class CAppManager : public IAppManager
{
private:
    IAppList apps;
    CBaseApp *defaultApp;

    CBaseApp *checkApp(const CBaseApp &app);
    void switchApp(CBaseApp *from, CBaseApp *to);

    enum EventType
    {
        Shutdown,
        ActivateApp,
        ActivateDefaultApp,
        PreviousApp,
    };

    struct CEvent
    {
        EventType eventType;
        CBaseApp *targetApp;
    };

    typedef std::queue<CEvent> CEvents;
    CEvents events;
    std::mutex eventsMutex;
    std::condition_variable eventsSignal;
    void sendEvent(EventType eventType, CBaseApp *targetApp);

    std::thread worker;
    std::mutex workerMutex;
    void work();

public:
    CAppManager();

    void init();
    void deinit();

    void registerApp(CBaseApp &app) override;
    [[nodiscard]] const IAppList &getApps() const override;

    void setDefaultApp(const CBaseApp &app);
    void activateApp(const CBaseApp &app) override;
    void previousApp() override;
    void activateDefaultApp();

    void start();
    void stop();
};

} // namespace Fri3d::Application
