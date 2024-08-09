#pragma once

#include <memory>

#include "nvs.h"

namespace Fri3d::Application
{

/**
 * @brief the handle will auto-convert to nvs_handle_t and will commit and close when it goes out of scope.
 * While you can call `nvs_commit()` on the handle yourself, you should never call `nvs_close()` on it.
 *
 */
class CNvsHandle
{
private:
    std::shared_ptr<nvs_handle_t> handle;
    std::string ns;

    explicit CNvsHandle(const char *ns);
    friend class INvsManager;

public:
    CNvsHandle(const CNvsHandle &other);
    ~CNvsHandle();

    CNvsHandle &operator=(const CNvsHandle &other);
    operator nvs_handle_t(); // NOLINT(google-explicit-constructor)

    /**
     * @brief convenience function to fetch strings and auto-convert them to std::string
     *
     * @param key
     * @return std::string containing the value, empty string if the key does not exist or is empty
     */
    std::string getString(const char *key);
};

class INvsManager
{
public:
    /**
     * @brief open the specified namespace
     *
     * @param ns namespace to open
     *
     * @return an open handle
     */
    virtual CNvsHandle open(const char *ns);

    /**
     * @brief open the
     * @return
     */
    virtual CNvsHandle openSys() = 0;
};

} // namespace Fri3d::Application
