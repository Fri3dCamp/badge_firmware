#pragma once

#include <list>

#include "lvgl.h"

#include "fri3d_application/app.hpp"
#include "fri3d_application/app_manager.hpp"

namespace Fri3d::Apps::Launcher
{

class CLauncher : public Application::CBaseApp
{
private:
    typedef const Application::IAppManager::IAppList &IAppList;
    lv_obj_t *screen;

    bool splashShown;

    void show(IAppList apps);
    void hide();

    static const CBaseApp *findSplash(IAppList apps);

    // As we need to pass multiple references to the buttons being clicked, we pair them together and keep references to
    // them so we can clean them up when necessary
    typedef std::pair<Application::IAppManager &, const Application::CBaseApp *> CEventData;
    typedef std::list<CEventData> CAppsEventData;
    CAppsEventData eventData;

    static void clickEvent(lv_event_t *event);

public:
    CLauncher();

    void init() override;
    void deinit() override;

    [[nodiscard]] const char *getName() const override;
    [[nodiscard]] bool getVisible() const override;

    void activate() override;
    void deactivate() override;
};

} // namespace Fri3d::Apps::Launcher
