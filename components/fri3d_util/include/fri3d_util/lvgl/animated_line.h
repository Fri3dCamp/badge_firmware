#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    lv_style_t *style;
    lv_point_precise_t *points;
    uint32_t duration;
    uint32_t delay;
    lv_anim_path_cb_t path_cb;
} fri3d_lv_animated_line_config_t;

/**
 * @brief draw a line with an animation, making it look like the line gets drawn from `from` to `to`
 *
 * Note that this function does not call `lv_lock()`, it expects it to be called already by the caller
 *
 * @param parent parent LVGL object
 * @param config configuration for the line. The points array in the config should exist as long as the line exists as
 * it is used by the lv_line_t. Note that the contents gets changed while the animation is playing.
 *
 * @returns the line that gets created with `lv_line_create()`
 */
lv_obj_t *fri3d_lv_animated_line(lv_obj_t *parent, fri3d_lv_animated_line_config_t config);

#ifdef __cplusplus
}
#endif
