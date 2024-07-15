#pragma once

#include "fri3d_application/app_manager.hpp"

namespace Fri3d::Application
{

class IApplication
{
public:
    /**
     * @brief Initialize the application
     */
    virtual void init() = 0;

    /**
     * @brief Deinitialize the application and release all the resources associated with it.
     */
    virtual void deinit() = 0;

    /**
     * @return the App Manager
     */
    virtual IAppManager &getAppManager() = 0;

    /**
     * @brief run the application loop until completion. The passed app is considered the main app and will also be
     * activated whenever the 'Menu' button is pushed
     *
     * @param[in]app the main app
     */
    virtual void run(const CBaseApp &app) = 0;
};

extern IApplication &application;

}
