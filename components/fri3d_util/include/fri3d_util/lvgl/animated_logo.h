#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// typedef struct
//{
//     lv_style_t *style;
//     lv_point_precise_t *points;
//     uint32_t duration;
//     lv_anim_path_cb_t path_cb;
// } fri3d_lv_animated_logo_t;
//
///**
// * @brief draw a line with an animation, making it look like the line gets drawn from `from` to `to`
// *
// * Note that this function does not call `lv_lock()`, it expects it to be called already by the caller
// *
// * @param parent parent LVGL object
// * @param config configuration for the line. This struct should exist for as long as the returned line exists as the
// * points array is used by the lv_line_t
// *
// * @returns the line that gets created with `lv_line_create()`
// */
// lv_obj_t *fri3d_lv_animated_line(lv_obj_t *parent, fri3d_lv_animated_line_t config);

/**
 * @brief display an animated Fri3d logo
 *
 * Note that for simplicity of implementation only 1 logo can be displayed at the same time. The animation will start as
 * soon as this function is called
 *
 * Width and height need to be passed beforehand because of the way animations work in LVGL, could be improved
 *
 * @param parent parent LVGL object
 * @param width width of the logo
 * @param height height of the logo
 * @return an LVGL object that contains all the other drawing components
 */
lv_obj_t *fri3d_lv_animated_logo_create(lv_obj_t *parent, int32_t width, int32_t height);

#ifdef __cplusplus
}
#endif
