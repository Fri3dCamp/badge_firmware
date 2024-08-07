#pragma once

#include "fri3d_application/app_manager.hpp"
#include "fri3d_application/thread.hpp"

namespace Fri3d::Application
{

// clang-format off
EVENT_CREATE_START(AppManagerEvent)
EVENT_CREATE_TYPES_START()
    ActivateApp,
    ActivateDefaultApp,
    PreviousApp,
EVENT_CREATE_TYPES_END()
    CBaseApp *targetApp;
EVENT_CREATE_END()
// clang-format on

class CAppManager : public CThread<AppManagerEvent>, public IAppManager
{
private:
    typedef std::vector<CBaseApp *> NavigationList;

    IAppList apps;
    CBaseApp *defaultApp;

    CBaseApp *checkApp(const CBaseApp &app);
    static void switchApp(CBaseApp *from, CBaseApp *to);

    // Pointers to other managers to store in the apps
    IHardwareManager *hardwareManager;
    INvsManager *nvsManager;

    void onEvent(const AppManagerEvent &event) override;

    NavigationList navigation;

public:
    CAppManager();

    void init(IHardwareManager &hardware, INvsManager &nvs);
    void deinit();

    void registerApp(CBaseApp &app) override;
    [[nodiscard]] const IAppList &getApps() const override;

    void setDefaultApp(const CBaseApp &app);
    void activateApp(const CBaseApp &app) override;
    void previousApp() override;
    void activateDefaultApp();
};

} // namespace Fri3d::Application
