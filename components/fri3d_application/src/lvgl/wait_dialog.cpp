#include "fri3d_application/lvgl/wait_dialog.hpp"
#include "fri3d_application/lvgl.hpp"

namespace Fri3d::Application::LVGL
{

CWaitDialog::CWaitDialog(const char *status)
    : text(status)
    , dialog(nullptr)
    , label(nullptr)
{
}

void CWaitDialog::setStatus(const char *status)
{
    this->text = status;

    if (this->label != nullptr)
    {
        lv_lock();
        lv_label_set_text(this->label, this->text);
        lv_unlock();
    }
}

void CWaitDialog::show()
{
    if (this->dialog != nullptr)
    {
        return;
    }

    lv_lock();

    // We draw over the active screen
    lv_obj_t *screen = lv_screen_active();

    // Create the main dialog
    this->dialog = lv_obj_create(screen);
    lv_obj_set_size(this->dialog, LV_PCT(80), LV_PCT(80));
    lv_obj_center(this->dialog);
    lv_obj_set_flex_flow(this->dialog, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(this->dialog, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(this->dialog, LV_OPA_90, LV_PART_MAIN);

    // Make sure no other inputs are possible by stealing and freezing it
    lv_group_add_obj(lv_group_get_default(), this->dialog);
    lv_group_focus_obj(this->dialog);
    lv_group_focus_freeze(lv_group_get_default(), true);

    // Create rest of widgets
    auto spinner = lv_spinner_create(this->dialog);
    lv_spinner_set_anim_params(spinner, 1000, 300);
    lv_obj_set_size(spinner, 100, 100);
    lv_obj_set_flex_grow(spinner, 1);

    this->label = lv_label_create(this->dialog);
    lv_label_set_text(this->label, this->text);

    lv_unlock();
}

void CWaitDialog::hide()
{
    if (this->dialog != nullptr)
    {
        lv_lock();

        lv_obj_delete(this->dialog);

        this->dialog = nullptr;
        this->label = nullptr;

        lv_unlock();
    }
}

CWaitDialog::~CWaitDialog()
{
    this->hide();
}

} // namespace Fri3d::Application::LVGL
