#pragma once

#include <map>
#include <string>
#include <vector>

#include "fri3d_application/app.hpp"
#include "fri3d_application/thread.hpp"
#include "fri3d_private/firmware_fetcher.hpp"

namespace Fri3d::Apps::Ota
{

// clang-format off
EVENT_CREATE_START(OtaEvent)
EVENT_CREATE_TYPES_START()
    FetchFirmwares,
    ToggleBeta,
    SelectedFirmware,
    UpdatePreview,
    UpdateFirmware,
    CancelUpdate,
EVENT_CREATE_TYPES_END()
EVENT_CREATE_END();
// clang-format on

class COta : public Application::CBaseApp, public Application::CThread<OtaEvent>
{
private:
    std::map<CImage::ImageType, CVersion> currentVersions;
    CVersion currentFirmware;
    lv_obj_t *screen;
    CFirmwareFetcher fetcher;
    CFirmware selectedFirmware;
    bool showBeta;

    std::optional<bool> updateMain;
    std::optional<bool> updateMicroPython;
    std::optional<bool> updateRetroGo;
    std::optional<bool> updateVfs;

    void hide();
    void showVersions();
    void showUpdate();

    static void onClickExit(lv_event_t *event);
    static void onClickFetchVersions(lv_event_t *event);
    static void onClickCancel(lv_event_t *event);
    static void onClickPreview(lv_event_t *event);
    static void onClickUpdate(lv_event_t *event);
    static void onVersionChange(lv_event_t *event);
    static void onCheckboxBetaToggle(lv_event_t *event);

    void onEvent(const OtaEvent &event) override;

    std::string drop_down_options; // all the options from available_versions concatenated with '\n' as separator

    bool ensureWifi();
    void fetchFirmwares();
    void updateFirmware();

    static void onImageCheckboxToggle(lv_event_t *event);
    void addImageCheckbox(
        lv_obj_t *container,
        std::optional<bool> &active,
        CImage::ImageType imageType,
        bool enabled = true);

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
