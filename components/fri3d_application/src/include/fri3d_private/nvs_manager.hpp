#pragma once

#include <map>
#include <memory>
#include <string>

#include "fri3d_application/nvs_manager.hpp"

namespace Fri3d::Application
{

class CNvsManager : public INvsManager
{
private:
    typedef std::map<std::string, CNvsHandle> NvsHandles;
    NvsHandles handles;

public:
    CNvsManager();

    void init();
    void deinit();

    CNvsHandle open(const char *ns) override;
    CNvsHandle openSys() override;
};

} // namespace Fri3d::Application
