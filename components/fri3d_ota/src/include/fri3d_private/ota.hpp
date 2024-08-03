#pragma once

#include <map>
#include <string>
#include <vector>

#include "fri3d_application/app.hpp"
#include "fri3d_application/thread.hpp"
#include "fri3d_private/ota_helper.h"
#include "fri3d_private/version_helper.h"

namespace Fri3d::Apps::Ota
{

struct ota_version_t
{
    std::string version;
    std::string url;
    int size;
};

// clang-format off
EVENT_CREATE_START(OtaEvent)
EVENT_CREATE_TYPES_START()
    FetchVersions,
    SelectedVersion,
    Update,
EVENT_CREATE_TYPES_END()
EVENT_CREATE_END();
// clang-format on

class COta : public Application::CBaseApp, public Application::CThread<OtaEvent>
{
private:
    const char *currentVersion;
    lv_obj_t *screen;

    typedef std::vector<ota_version_t> CVersions;
    CVersions versions;
    ota_version_t selectedVersion;

    void hide();
    void showVersions();

    static void onClickExit(lv_event_t *event);
    static void onClickFetchVersions(lv_event_t *event);
    static void onClickUpdate(lv_event_t *event);
    static void onVersionChange(lv_event_t *event);

    void onEvent(const OtaEvent &event) override;

    std::string drop_down_options; // all the options from available_versions concatenated with '\n' as separator

    void do_fetch_versions();
    void do_upgrade();

public:
    COta();

    void init() override;
    void deinit() override;

    [[nodiscard]] const char *getName() const override;
    [[nodiscard]] bool getVisible() const override;

    void activate() override;
    void deactivate() override;
};

} // namespace Fri3d::Apps::Ota
