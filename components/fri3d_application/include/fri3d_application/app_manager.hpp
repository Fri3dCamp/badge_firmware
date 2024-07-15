#pragma once

#include <vector>

#include "app.hpp"

namespace Fri3d::Application
{

class IAppManager
{
public:
    typedef std::vector<CBaseApp *> IAppList;

    /**
     * @brief register a new app in the manager
     *
     * @param app
     */
    virtual void registerApp(CBaseApp &app) = 0;

    /**
     * @return the list of registered apps
     */
    [[nodiscard]] virtual const IAppList &getApps() const = 0;

    /**
     * @brief activates the referenced app
     *
     * @param app
     */
    virtual void activateApp(const CBaseApp &app) = 0;

    /**
     * @brief activate the previous app, deactivating the current app. Note that an app can also use this to actually
     * deactivate itself, for example when it's done processing something.
     */
    virtual void previousApp() = 0;
};

}
