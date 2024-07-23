#include <chrono>
#include <thread>

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "freertos/FreeRTOS.h"

#include "fri3d_application/partition_boot.hpp"
#include "fri3d_private/app_manager.hpp"

using namespace std::literals;

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CPartitionBoot";

CPartitionBoot::CPartitionBoot(const char *name, const char *partition)
    : name(name)
    , partition(partition)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

const char *CPartitionBoot::getName() const
{
    return name;
}

bool CPartitionBoot::getVisible() const
{
    return true;
}

void CPartitionBoot::init()
{
}

void CPartitionBoot::deinit()
{
}

void CPartitionBoot::activate()
{
    ESP_LOGI(TAG, "Switching to `%s`", this->partition);

    const esp_partition_t *next_boot_partition =
        esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, this->partition);
    if (next_boot_partition == NULL)
    {
        ESP_LOGE(TAG, "Failed getting `%s` partition", this->partition);
    }
    else
    {
        ESP_LOGV(
            TAG,
            "Setting boot partition of type: %d, subtype: %d, address: %lu, size: %lu, label: %s",
            next_boot_partition->type,
            next_boot_partition->subtype,
            (uint32_t)next_boot_partition->address,
            (uint32_t)next_boot_partition->size,
            next_boot_partition->label);
        esp_err_t err = esp_ota_set_boot_partition(next_boot_partition);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "esp_ota_set_boot_partition returned 0x%02X!\n", err);
        }
        else
        {
            ESP_LOGI(TAG, "Booting into %s", this->partition);
            // should we display something on the screen?
            std::this_thread::sleep_for(300ms);

            esp_restart();
        }
    }

    // Something went wrong
    this->getAppManager().previousApp();
}

void CPartitionBoot::deactivate()
{
}

} // namespace Fri3d::Application
