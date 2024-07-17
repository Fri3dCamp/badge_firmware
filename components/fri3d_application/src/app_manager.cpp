#include <algorithm>
#include <exception>

#include "esp_log.h"

#include "fri3d_private/app.hpp"
#include "fri3d_private/app_manager.hpp"

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CAppManager";

CAppManager::CAppManager()
    : defaultApp(nullptr)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CAppManager::init()
{
    ESP_LOGI(TAG, "Initializing");
}

void CAppManager::deinit()
{
    ESP_LOGI(TAG, "Deinitializing all apps");
    for (CBaseApp *app : this->apps)
    {
        app->deinit();
    }

    ESP_LOGI(TAG, "Deinitializing");
    this->apps = std::vector<CBaseApp *>();
}

void CAppManager::registerApp(CBaseApp &app)
{
    ESP_LOGI(TAG, "Registering new app (%s)", app.getName());

    app.base->setAppManager(this);
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

    auto it = std::find_if(this->apps.begin(), this->apps.end(), [ref](CBaseApp *a) { return a == ref; });

    if (it == this->apps.end())
    {
        throw std::runtime_error("App is not registred.");
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

    this->sendEvent(EventType::ActivateApp, ref);
}

void CAppManager::activateDefaultApp()
{
    if (!this->defaultApp)
    {
        ESP_LOGW(TAG, "Default app not set");
        return;
    }

    this->sendEvent(EventType::ActivateDefaultApp, this->defaultApp);
}

void CAppManager::previousApp()
{
    this->sendEvent(EventType::PreviousApp, nullptr);
}

void CAppManager::start()
{
    std::lock_guard lock(this->workerMutex);
    if (this->worker.joinable())
    {
        throw std::runtime_error("Already running");
    }

    // Make sure the event queue is empty before we start
    this->events = CEvents();

    this->worker = std::thread(&CAppManager::work, this);
}

void CAppManager::stop()
{
    std::lock_guard lock(this->workerMutex);
    if (this->worker.joinable())
    {
        sendEvent(EventType::Shutdown, nullptr);
        this->worker.join();
        this->events = CEvents();
    }
}

void CAppManager::work()
{
    bool running = true;
    IAppList navigation;
    std::unique_lock lock(eventsMutex);

    while (running)
    {
        eventsSignal.wait(lock);

        while (!this->events.empty())
        {
            auto event = this->events.front();
            this->events.pop();

            auto previous = navigation.empty() ? nullptr : navigation.back();

            switch (event.eventType)
            {
            case EventType::Shutdown:
                if (previous)
                {
                    previous->deactivate();
                }
                running = false;
                break;

            case ActivateDefaultApp:
                ESP_LOGD(TAG, "Default app activated, cleaning navigation history.");
                navigation = IAppList();

                // We fall through to app activation
                [[fallthrough]];

            case ActivateApp:
                navigation.push_back(event.targetApp);
                this->switchApp(previous, event.targetApp);
                break;

            case PreviousApp:
                if (navigation.size() > 1)
                {
                    navigation.pop_back();
                    this->switchApp(previous, navigation.back());
                }
                break;
            }
        }
    }
}

void CAppManager::sendEvent(CAppManager::EventType eventType, CBaseApp *targetApp)
{
    ESP_LOGV(TAG, "Sending event (eventType: %d; targetApp: %p)", eventType, targetApp);
    {
        std::lock_guard lock(this->eventsMutex);
        this->events.push(CEvent{.eventType = eventType, .targetApp = targetApp});
    }

    this->eventsSignal.notify_all();
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

} // namespace Fri3d::Application
