#pragma once

#include <experimental/propagate_const>
#include <memory>

namespace Fri3d::Application
{

class IAppManager;
class CAppManager;

// Base class all apps should inherit from
class CBaseApp
{
private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> base;

    friend class CAppManager;
public:
    CBaseApp();
    ~CBaseApp();

    /**
     * @brief access the App Manager
     *
     * @return the app manager instance
     */
    [[nodiscard]] IAppManager &getAppManager() const;

    /**
     * @brief called upon registration in the app_manager, allows the app to do some initialization beforehand.
     * The app should not start performing tasks yet, unless it is a background service.
     *
     */
    virtual void init() = 0;

    /**
     * @brief the app should stop all tasks and release all resources immediately
     */
    virtual void deinit() = 0;

    /**
     * @brief get the name of the app
     *
     * @return name of the app
     */
    [[nodiscard]] virtual const char *getName() const = 0;

    /**
     * @brief the app has been activated (brought to the foreground), it should start doing something.
     * Note that this function should return asap, any required processing should be done in a separate thread
     */
    virtual void activate() = 0;

    /**
     * @brief the app has been deactivated, it should stop working immediately as control will be handed over to
     * another app after this call
     */
    virtual void deactivate() = 0;
};

}
