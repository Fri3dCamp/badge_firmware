#pragma once

#include "fri3d_private/firmware.hpp"

namespace Fri3d::Apps::Ota
{

class CFlasher
{
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
