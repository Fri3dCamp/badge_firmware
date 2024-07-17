#pragma once

#include "fri3d_application/app.hpp"

namespace Fri3d::Application
{

class CBaseApp::impl
{
private:
    IAppManager *appManager;

public:
    void setAppManager(IAppManager *value);
    [[nodiscard]] IAppManager &getAppManager() const;
};

} // namespace Fri3d::Application
