#pragma once

#include <map>
#include <vector>

#include "fri3d_application/app.hpp"
#include "fri3d_private/ota_helper.h"
#include "fri3d_private/version_helper.h"

namespace Fri3d::Apps::Ota
{

struct ota_version_t
{
    char *version;
    std::vector<ota_file_t> files;
};

class COta : public Application::CBaseApp
{
private:
    enum ActionType
    {
        NoAction,
        CheckOnline,
        Upgrade,
    };

    ActionType action;

    lv_obj_t *btn_check;
    lv_obj_t *cont_spinner;
    lv_obj_t *spinner_label;
    lv_obj_t *progress_bar;
    lv_obj_t *cont_row;
    lv_obj_t *cont_col;
    lv_obj_t *label_error = nullptr;

    const char *current_version;
    std::string drop_down_options; // all the options from available_versions concatenated with '\n' as separator
    ota_version_t selected_version;
    std::vector<ota_version_t> available_versions;

    static const char *getBoardName();
    void screen_initial_layout();
    void screen_spinner_start(const char *label_text);
    void screen_spinner_stop();
    void screen_spinner_add_progress_bar_message(const char *progress_message);
    void screen_spinner_set_progress_bar_value(int32_t value);
    void screen_update_available_versions();
    void screen_error_label_show(const char *);
    void screen_error_label_remove();

    static void callback_btn_cancel_click(lv_event_t *e);
    static void callback_btn_check_click(lv_event_t *e);
    static void callback_drop_down_change(lv_event_t *e);
    static void callback_btn_update_click(lv_event_t *e);

    void do_fetch_versions(void);
    version_task_parameters_t version_task_parameters;
    static int path_to_version(const cJSON *path, const char *ota_part, char **version);
    static int path_to_version_filename(const cJSON *path, const char *ota_part, char **version, char **filename);

    upgrade_task_parameters_t upgrade_task_parameters;
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