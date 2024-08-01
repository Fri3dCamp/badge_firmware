#pragma once

#include "lvgl.h"

namespace Fri3d::Application::LVGL
{

/**
 * @brief a modal waiting dialog that can be shown and hidden
 */
class CWaitDialog
{
private:
    const char *text;
    lv_obj_t *dialog;
    lv_obj_t *label;

public:
    explicit CWaitDialog(const char *status);
    ~CWaitDialog();

    void setStatus(const char *status);

    void show();
    void hide();
};

} // namespace Fri3d::Application::LVGL
