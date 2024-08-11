#pragma once

#include "esp_http_client.h"
#include "esp_ota_ops.h"

#include "fri3d_application/lvgl/wait_dialog.hpp"
#include "fri3d_private/firmware.hpp"

namespace Fri3d::Apps::Ota
{

class CFlasher
{
private:
    static bool flashOta(
        const CImage &image,
        const esp_partition_t *partition,
        esp_http_client_config_t &httpConfig,
        Application::LVGL::CWaitDialog &dialog);

    static bool flashRaw(
        const CImage &image,
        const esp_partition_t &partition,
        esp_http_client_config_t &httpConfig,
        Application::LVGL::CWaitDialog &dialog);

    struct RawUserData
    {
        const esp_partition_t &partition;
        int size;
        int contentLength;
        Application::LVGL::CWaitDialog &dialog;
        size_t written;
    };

    static void setStatusFlashing(const CImage &image, Application::LVGL::CWaitDialog &dialog);
    static void setStatusErasing(const esp_partition_t &partition, Application::LVGL::CWaitDialog &dialog);

public:
    CFlasher();

    /**
     * @brief persist the main firmware image after its first boot
     *
     * @returns name of the persisted partition
     */
    static std::string persist();

    /**
     * @brief flash the image to one of the two main firmware partitions
     * @param image
     *
     * @returns true on success
     */
    static bool flash(const CImage &image);

    /**
     * @brief flash the image to the specified partition
     * @param image
     * @param partitionName can be nullptr, in which case main firmware partitions are used
     *
     * @returns true on success
     */
    static bool flash(const CImage &image, const char *partitionName);
};

} // namespace Fri3d::Apps::Ota
