#pragma once

#include "fri3d_private/version.hpp"

namespace Fri3d::Apps::Ota
{

class CFlasher
{
public:
    CFlasher();

    /**
     * @brief persist the main firmware image after its first boot
     */
    static void persist();

    /**
     * @brief flash the image to one of the two main firmware partitions
     * @param image
     *
     * @returns true on success
     */
    static bool flash(const CFirmwareVersion &image);

    /**
     * @brief flash the image to the specified partition
     * @param image
     * @param partitionName can be nullptr, in which case main firmware partitions are used
     *
     * @returns true on success
     */
    static bool flash(const CFirmwareVersion &image, const char *partitionName);
};

} // namespace Fri3d::Apps::Ota
