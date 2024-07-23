#pragma once

#include "fri3d_application/app.hpp"

namespace Fri3d::Application
{

class CPartitionBoot : public CBaseApp
{
private:
    const char *name;
    const char *partition;

public:
    CPartitionBoot(const char *name, const char *partition);

    void init() override;
    void deinit() override;

    void activate() override;
    void deactivate() override;

    [[nodiscard]] const char *getName() const override;
    [[nodiscard]] bool getVisible() const override;
};

} // namespace Fri3d::Application
