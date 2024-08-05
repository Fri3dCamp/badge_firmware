#include "fri3d_application/lvgl/wait_dialog.hpp"
#include "fri3d_application/lvgl.hpp"

namespace Fri3d::Application::LVGL
{

CWaitDialog::CWaitDialog(const char *status)
    : text(status)
    , dialog(nullptr)
    , label(nullptr)
    , spinnerContainer(nullptr)
    , spinner(nullptr)
    , progress(nullptr)
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

    // Create a box for the spinner, so we can easily replace it in setProgress if needed
    this->spinnerContainer = lv_obj_create(this->dialog);
    lv_obj_remove_style_all(this->spinnerContainer);
    lv_obj_set_flex_grow(this->spinnerContainer, 1);

    this->spinner = lv_spinner_create(this->spinnerContainer);
    lv_spinner_set_anim_params(this->spinner, 1000, 300);
    lv_obj_set_size(this->spinner, 120, 120);
    lv_obj_align(this->spinner, LV_ALIGN_TOP_MID, 0, 0);

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
        this->spinnerContainer = nullptr;
        this->spinner = nullptr;
        this->progress = nullptr;

        lv_unlock();
    }
}

CWaitDialog::~CWaitDialog()
{
    this->hide();
}

void CWaitDialog::setProgress(float value)
{
    lv_lock();

    if (this->spinner == nullptr)
    {
        return;
    }

    if (this->progress == nullptr)
    {
        // We haven't been called yet, replace the spinner and create the label
        lv_obj_delete(this->spinner);
        this->spinner = lv_arc_create(this->spinnerContainer);
        lv_obj_set_size(this->spinner, 120, 120);
        lv_obj_align(this->spinner, LV_ALIGN_TOP_MID, 0, 0);

        lv_arc_set_rotation(this->spinner, 270);
        lv_arc_set_bg_angles(this->spinner, 0, 360);
        lv_obj_remove_style(this->spinner, nullptr, LV_PART_KNOB);
        lv_obj_remove_flag(this->spinner, LV_OBJ_FLAG_CLICKABLE);

        this->progress = lv_label_create(this->spinner);
        lv_obj_center(this->progress);
    }

    lv_label_set_text_fmt(this->progress, "%03.2f %%", value);
    lv_arc_set_value(this->spinner, static_cast<int32_t>(value));

    lv_unlock();
}

} // namespace Fri3d::Application::LVGL
