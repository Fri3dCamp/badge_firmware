
#include <fri3d_application/nvs_manager.hpp>

#include "esp_log.h"
#include "nvs_flash.h"

#include "fri3d_private/nvs_manager.hpp"

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CNvsManager";

CNvsHandle::CNvsHandle(const char *ns)
    : handle(std::make_shared<nvs_handle_t>())
    , ns(ns)
{
}

CNvsHandle::CNvsHandle(const CNvsHandle &other)
{
    *this = other;
}

CNvsHandle::~CNvsHandle()
{
    // We commit anytime we go out of scope
    ESP_ERROR_CHECK(nvs_commit(*this->handle));

    // There's one reference always active in the map in CNvsManager. After this object is destructed, that's the only
    // one left, so we can close the handle
    if (this->handle.use_count() == 2)
    {
        // We are the last one using the handle, close it
        nvs_close(*this->handle);
    }
}

CNvsHandle::operator nvs_handle_t()
{
    return *this->handle;
}

CNvsHandle &CNvsHandle::operator=(const CNvsHandle &other)
{
    if (&other != this)
    {
        this->handle = other.handle;

        if (this->handle.use_count() == 2)
        {
            // Somebody wants to use this NVS namespace, let's open it!
            ESP_ERROR_CHECK(nvs_open(this->ns.c_str(), NVS_READWRITE, this->handle.get()));
        }
    }

    return *this;
}

CNvsManager::CNvsManager()
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

void CNvsManager::init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_LOGI(TAG, "Initialized");
}

void CNvsManager::deinit()
{
    this->handles = NvsHandles();

    ESP_ERROR_CHECK(nvs_flash_deinit());

    ESP_LOGI(TAG, "Deinitialized");
}

CNvsHandle CNvsManager::open(const char *ns)
{
    auto handle = this->handles.find(ns);

    if (handle == this->handles.end())
    {
        return this->handles.emplace(std::make_pair(ns, INvsManager::open(ns))).first->second;
    }
    else
    {
        return handle->second;
    }
}

CNvsHandle CNvsManager::openSys()
{
    return this->open("fri3d.sys");
}

CNvsHandle INvsManager::open(const char *ns)
{
    // We don't do any checking here, we just return the constructed object to the inherited class
    return CNvsHandle(ns);
}

} // namespace Fri3d::Application
