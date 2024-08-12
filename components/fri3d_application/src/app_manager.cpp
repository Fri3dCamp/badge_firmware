#include <algorithm>

#include "esp_log.h"

#include "fri3d_private/app.hpp"
#include "fri3d_private/app_manager.hpp"

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CAppManager";

CAppManager::CAppManager()
    : CThread<AppManagerEvent>(TAG)
    , defaultApp(nullptr)
    , hardwareManager(nullptr)
    , nvsManager(nullptr)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CAppManager::init(IHardwareManager &hardware, INvsManager &nvs)
{
    ESP_LOGI(TAG, "Initializing");
    this->hardwareManager = &hardware;
    this->nvsManager = &nvs;
}

void CAppManager::deinit()
{
    ESP_LOGI(TAG, "Deinitializing all apps");
    for (auto app : this->apps)
    {
        const_cast<CBaseApp *>(app)->deinit();
    }

    ESP_LOGI(TAG, "Deinitializing");
    this->apps = std::vector<const CBaseApp *>();

    this->nvsManager = nullptr;
    this->hardwareManager = nullptr;
}

void CAppManager::registerApp(CBaseApp &app)
{
    ESP_LOGI(TAG, "Registering new app (%s)", app.getName());

    app.base->setAppManager(this);
    app.base->setHardwareManager(this->hardwareManager);
    app.base->setNvsManager(this->nvsManager);

    app.init();

    this->apps.push_back(&app);
}

const IAppManager::IAppList &CAppManager::getApps() const
{
    return this->apps;
}

CBaseApp *CAppManager::checkApp(const Fri3d::Application::CBaseApp &app)
{
    // These function take const references as apps can only obtain those from the App Manager. We however need the
    // non-const references as we need to operate on the apps. That's why we cast const away
    CBaseApp *ref = &const_cast<CBaseApp &>(app);

    auto it = std::find_if(this->apps.begin(), this->apps.end(), [ref](const CBaseApp *a) { return a == ref; });

    if (it == this->apps.end())
    {
        throw std::runtime_error("App is not registered.");
    }

    return ref;
}

void CAppManager::setDefaultApp(const CBaseApp &app)
{
    this->defaultApp = this->checkApp(app);
}

void CAppManager::activateApp(const CBaseApp &app)
{
    CBaseApp *ref = this->checkApp(app);

    this->sendEvent({AppManagerEvent::ActivateApp, ref});
}

void CAppManager::activateDefaultApp()
{
    if (!this->defaultApp)
    {
        ESP_LOGW(TAG, "Default app not set");
        return;
    }

    this->sendEvent({AppManagerEvent::ActivateDefaultApp, this->defaultApp});
}

void CAppManager::previousApp()
{
    this->sendEvent({AppManagerEvent::PreviousApp, nullptr});
}

void CAppManager::switchApp(CBaseApp *from, CBaseApp *to)
{
    if (from)
    {
        ESP_LOGD(TAG, "Deactivating app (%s)", from->getName());
        from->deactivate();
    }

    ESP_LOGD(TAG, "Activating app (%s)", to->getName());
    to->activate();
}

void CAppManager::onEvent(const AppManagerEvent &event)
{
    ESP_LOGV(TAG, "Received event (eventType: %d, targetApp: %p)", event.eventType, event.targetApp);
    auto previous = this->navigation.empty() ? nullptr : this->navigation.back();

    switch (event.eventType)
    {
    case AppManagerEvent::Shutdown:
        if (previous)
        {
            // If we are shutting down, make sure the last active app also shuts down properly
            previous->deactivate();
        }
        break;
    case AppManagerEvent::ActivateDefaultApp:
        ESP_LOGD(TAG, "Default app activated, cleaning navigation history.");
        this->navigation = NavigationList();

        // We fall through to app activation
        [[fallthrough]];

    case AppManagerEvent::ActivateApp:
        this->navigation.push_back(event.targetApp);
        CAppManager::switchApp(previous, event.targetApp);
        break;

    case AppManagerEvent::PreviousApp:
        if (this->navigation.size() > 1)
        {
            this->navigation.pop_back();
            CAppManager::switchApp(previous, this->navigation.back());
        }
        break;
    }
}

void CAppManager::notifyStartStop(bool start) const
{
    for (auto item : this->apps)
    {
        auto app = const_cast<CBaseApp *>(item);

        if (start)
        {
            app->onSystemStart();
        }
        else
        {
            app->onSystemStop();
        }
    }
}

} // namespace Fri3d::Application
