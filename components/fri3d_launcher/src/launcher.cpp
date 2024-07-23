#include <cstring>

#include "esp_log.h"

#include "fri3d_private/launcher.hpp"

namespace Fri3d::Apps::Launcher
{

static const char *TAG = "Fri3d::Apps::Launcher::CLauncher";

CLauncher::CLauncher()
    : screen(nullptr)
    , splashShown(false)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CLauncher::init()
{
    ESP_LOGI(TAG, "Initializing launcher");
    if (this->screen == nullptr)
    {
        lv_lock();
        this->screen = lv_obj_create(nullptr);
        lv_unlock();
    }
}

void CLauncher::deinit()
{
    ESP_LOGI(TAG, "Deinitializing launcher");
    this->hide();

    if (this->screen != nullptr)
    {
        lv_lock();
        lv_obj_delete(this->screen);
        this->screen = nullptr;
        lv_unlock();
    }
}

const char *CLauncher::getName() const
{
    return "Launcher";
}

void CLauncher::activate()
{
    ESP_LOGD(TAG, "Fetching registered apps");
    const auto &apps = this->getAppManager().getApps();

    ESP_LOGD(TAG, "Found %d apps", apps.size());

    if (!this->splashShown)
    {
        auto splash = this->findSplash(apps);

        this->splashShown = true;

        if (splash == nullptr)
        {
            ESP_LOGI(TAG, "Could not find Splash screen, ignoring");
        }
        else
        {
            this->getAppManager().activateApp(*splash);
            return;
        }
    }

    this->show(apps);
    ESP_LOGD(TAG, "Activated");
}

void CLauncher::deactivate()
{
    this->hide();
    ESP_LOGD(TAG, "Deactivated");
}

void CLauncher::show(CLauncher::IAppList apps)
{
    lv_lock();

    auto title = lv_label_create(this->screen);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    lv_label_set_text(title, "Fri3d Camp");

    auto flex = lv_obj_create(this->screen);
    lv_obj_set_size(flex, 180, 160);
    lv_obj_set_flex_flow(flex, LV_FLEX_FLOW_COLUMN);
    lv_obj_align_to(flex, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    for (auto app : apps)
    {
        if (!app->getVisible())
        {
            continue;
        }

        auto button = lv_button_create(flex);
        lv_obj_set_size(button, LV_PCT(100), LV_SIZE_CONTENT);

        auto &data = eventData.emplace_back(this->getAppManager(), app);
        lv_obj_add_event_cb(button, CLauncher::clickEvent, LV_EVENT_CLICKED, &data);

        auto label = lv_label_create(button);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label, app->getName());
    }

    lv_screen_load(this->screen);

    lv_unlock();
}

void CLauncher::hide()
{
    lv_lock();
    lv_obj_clean(this->screen);
    eventData = CAppsEventData();
    lv_unlock();
}

const Application::CBaseApp *CLauncher::findSplash(CLauncher::IAppList apps)
{
    const Application::CBaseApp *splash = nullptr;

    for (auto app : apps)
    {
        if (std::strcmp(app->getName(), "Splash") == 0)
        {
            splash = app;
            break;
        }
        ESP_LOGV(TAG, "Found %s", app->getName());
    }

    return splash;
}

void CLauncher::clickEvent(lv_event_t *event)
{
    auto data = static_cast<CEventData *>(lv_event_get_user_data(event));

    ESP_LOGI(TAG, "Switching to %s", data->second->getName());
    data->first.activateApp(*data->second);
}

bool CLauncher::getVisible() const
{
    return false;
}

static CLauncher launcher_impl;
Application::CBaseApp &launcher = launcher_impl;

} // namespace Fri3d::Apps::Launcher
