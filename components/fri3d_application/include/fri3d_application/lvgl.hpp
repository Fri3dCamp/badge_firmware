// This is a temporary workaround until LVGL releases a new version with these functions
// Meanwhile we can already write compatible code
// When LVGL is updated and these are removed, the lock around lv_timer_handler() in CLVGL::work() also needs to be
// removed.
#include "lvgl.h"

void lv_lock(void);
lv_result_t lv_lock_isr(void);
void lv_unlock(void);
