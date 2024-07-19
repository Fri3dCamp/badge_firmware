#include "fri3d_private/button.hpp"
#include "fri3d_application/lvgl.hpp"

namespace Fri3d::Apps::Launcher
{

CButton::CButton(int offset)
    : button(nullptr)
    , label(nullptr)
    , count(0)
    , offset(offset)
{
}

CButton::~CButton()
{
    this->hide();
}

void CButton::show()
{
    assert(!this->button);
    assert(!this->label);

    lv_lock();

    auto screen = lv_screen_active();

    this->button = lv_button_create(screen);
    lv_obj_align(this->button, LV_ALIGN_CENTER, 0, offset);
    lv_obj_add_event_cb(this->button, CButton::click, LV_EVENT_CLICKED, this);

    this->label = lv_label_create(this->button);

    this->setText();

    lv_unlock();
}

void CButton::hide()
{
    lv_lock();

    if (this->label)
    {
        lv_obj_delete(this->label);
        this->label = nullptr;
    }

    if (this->button)
    {
        lv_obj_delete(this->button);
        this->button = nullptr;
    }

    lv_unlock();
}

void CButton::setText()
{
    assert(this->label);

    // Note that we don't take a lock, either we have it from CButton::show(), or from the CButton::click()
    lv_label_set_text_fmt(this->label, "%d clicks", this->count);
}

void CButton::click(lv_event_t *event)
{
    auto self = static_cast<CButton *>(event->user_data);

    self->count++;
    self->setText();
}

} // namespace Fri3d::Apps::Launcher
