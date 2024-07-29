#include "fri3d_util/lvgl/animated_line.h"

static void animate_point_x(void *var, int32_t value)
{
    lv_obj_t *line = (lv_obj_t *)var;
    lv_point_precise_t *points = (lv_point_precise_t *)lv_obj_get_user_data(line);

    points[1].x = value;

    // We just set the same points array again, this forces the line to invalidate
    lv_line_set_points(line, points, 2);
}

static void animate_point_y(void *var, int32_t value)
{
    lv_obj_t *line = (lv_obj_t *)var;
    lv_point_precise_t *points = (lv_point_precise_t *)lv_obj_get_user_data(line);

    points[1].y = value;

    // We just set the same points array again, this forces the line to invalidate
    lv_line_set_points(line, points, 2);
}

lv_obj_t *fri3d_lv_animated_line(lv_obj_t *parent, fri3d_lv_animated_line_config_t config)
{
    // Naive implementation which just uses two animations. Could be possible with only one animation, but then we'd
    // need to implement the interpolation ourselves
    lv_obj_t *line = lv_line_create(parent);
    lv_point_precise_t *points = config.points;

    // Make sure the parent returns correct size
    lv_obj_update_layout(parent);
    int32_t parent_width = lv_obj_get_width(parent);
    int32_t parent_height = lv_obj_get_height(parent);

    // We need to explicitly set the width/height in case we are passed percentage values for the points
    lv_obj_set_size(line, parent_width, parent_height);

    lv_obj_add_style(line, config.style, 0);

    // Animate X change
    lv_anim_t a_x;
    lv_anim_init(&a_x);
    lv_anim_set_var(&a_x, line);
    lv_anim_set_values(&a_x, points[0].x, points[1].x);
    lv_anim_set_duration(&a_x, config.duration);
    lv_anim_set_delay(&a_x, config.delay);
    lv_anim_set_path_cb(&a_x, config.path_cb);
    // In LVGL 9.1 `lv_line_get_points_mutable()` is not yet available, so we pass the points as user data
    lv_obj_set_user_data(line, points);
    lv_anim_set_exec_cb(&a_x, animate_point_x);

    // Animate Y change
    lv_anim_t a_y;
    lv_anim_init(&a_y);
    lv_anim_set_var(&a_y, line);
    lv_anim_set_values(&a_y, points[0].y, points[1].y);
    lv_anim_set_duration(&a_y, config.duration);
    lv_anim_set_delay(&a_y, config.delay);
    lv_anim_set_path_cb(&a_y, config.path_cb);
    // In LVGL 9.1 `lv_line_get_points_mutable()` is not yet available, so we pass the points as user data
    lv_obj_set_user_data(line, points);
    lv_anim_set_exec_cb(&a_y, animate_point_y);

    // We set the line to 0 to make sure we don't get any drawing artifacts and only now pass it to the line object
    points[1].x = points[0].x;
    points[1].y = points[0].y;

    lv_line_set_points(line, config.points, 2);

    // Start the animations
    lv_anim_start(&a_x);
    lv_anim_start(&a_y);

    return line;
}
