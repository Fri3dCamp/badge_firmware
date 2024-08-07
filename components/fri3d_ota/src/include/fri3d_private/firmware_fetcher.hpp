#pragma once

#include <string>

#include "fri3d_private/firmware.hpp"

namespace Fri3d::Apps::Ota
{

class CFirmwareFetcher
{
private:
    CFirmwares firmwares;
    CFirmwares official;

    static std::string fetch();
    bool parse(const char *json);

public:
    CFirmwareFetcher();

    [[nodiscard]] bool refresh();
    [[nodiscard]] const CFirmwares &getFirmwares(bool beta) const;
};

} // namespace Fri3d::Apps::Ota
