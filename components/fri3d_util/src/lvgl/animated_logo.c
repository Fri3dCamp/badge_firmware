#include <math.h>

#include "esp_heap_caps.h"
#include "esp_log.h"

#include "fri3d_util/lvgl/animated_line.h"
#include "fri3d_util/lvgl/animated_logo.h"

// Points of the logo
#define FRI3D_LOGO_TOP_RIGHT_X LV_PCT(96)
#define FRI3D_LOGO_TOP_RIGHT_Y LV_PCT(8)
#define FRI3D_LOGO_TOP_RIGHT                                                                                           \
    {                                                                                                                  \
        FRI3D_LOGO_TOP_RIGHT_X, FRI3D_LOGO_TOP_RIGHT_Y                                                                 \
    }
#define FRI3D_LOGO_TOP_LEFT_X LV_PCT(4)
#define FRI3D_LOGO_TOP_LEFT_Y LV_PCT(8)
#define FRI3D_LOGO_TOP_LEFT                                                                                            \
    {                                                                                                                  \
        FRI3D_LOGO_TOP_LEFT_X, FRI3D_LOGO_TOP_LEFT_Y                                                                   \
    }
#define FRI3D_LOGO_MIDDLE_RIGHT_X LV_PCT(100)
#define FRI3D_LOGO_MIDDLE_RIGHT_Y LV_PCT(40)
#define FRI3D_LOGO_MIDDLE_RIGHT                                                                                        \
    {                                                                                                                  \
        FRI3D_LOGO_MIDDLE_RIGHT_X, FRI3D_LOGO_MIDDLE_RIGHT_Y                                                           \
    }
#define FRI3D_LOGO_MIDDLE_LEFT_X LV_PCT(0)
#define FRI3D_LOGO_MIDDLE_LEFT_Y LV_PCT(40)
#define FRI3D_LOGO_MIDDLE_LEFT                                                                                         \
    {                                                                                                                  \
        FRI3D_LOGO_MIDDLE_LEFT_X, FRI3D_LOGO_MIDDLE_LEFT_Y                                                             \
    }
#define FRI3D_LOGO_BOTTOM_X LV_PCT(50)
#define FRI3D_LOGO_BOTTOM_Y LV_PCT(92)
#define FRI3D_LOGO_BOTTOM                                                                                              \
    {                                                                                                                  \
        FRI3D_LOGO_BOTTOM_X, FRI3D_LOGO_BOTTOM_Y                                                                       \
    }
#define FRI3D_LOGO_CROSS_LEFT_X LV_PCT(48)
#define FRI3D_LOGO_CROSS_LEFT_Y LV_PCT(25)
#define FRI3D_LOGO_CROSS_LEFT                                                                                          \
    {                                                                                                                  \
        FRI3D_LOGO_CROSS_LEFT_X, FRI3D_LOGO_CROSS_LEFT_Y                                                               \
    }
#define FRI3D_LOGO_CROSS_RIGHT_X LV_PCT(52)
#define FRI3D_LOGO_CROSS_RIGHT_Y LV_PCT(25)
#define FRI3D_LOGO_CROSS_RIGHT                                                                                         \
    {                                                                                                                  \
        FRI3D_LOGO_CROSS_RIGHT_X, FRI3D_LOGO_CROSS_RIGHT_Y                                                             \
    }

#define FRI3D_LOGO_CIRCLES 10

static void fri3d_clear_buffer(lv_event_t *event)
{
    lv_draw_buf_t *buffer = (lv_draw_buf_t *)lv_event_get_user_data(event);
    lv_draw_buf_destroy(buffer);
}

static void animate_opa(void *var, int32_t value)
{
    lv_obj_set_style_opa((lv_obj_t *)var, value, 0);
}

static void animate_circle(void *var, int32_t value)
{
    lv_obj_set_size((lv_obj_t *)var, value, value);
}

static lv_obj_t *create_circle(lv_obj_t *parent, int32_t size)
{
    lv_obj_t *arc = lv_arc_create(parent);
    lv_arc_set_angles(arc, 0, 360);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_remove_style(arc, NULL, LV_PART_MAIN);
    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(arc, 3, LV_PART_INDICATOR);
    lv_obj_set_size(arc, size, size);
    lv_obj_center(arc);

    return arc;
}

lv_obj_t *fri3d_lv_animated_logo_create(lv_obj_t *parent, int32_t width, int32_t height)
{
    lv_color_t background_color = lv_obj_get_style_bg_color(parent, LV_PART_MAIN);
    int32_t square_xy = (width < height ? width : height) - 20;
    int32_t diag = (int32_t)sqrt(width * width + height * height);

    /* Container */
    lv_obj_t *widget = lv_obj_create(parent);
    lv_obj_remove_style_all(widget);
    lv_obj_set_size(widget, width, height);

    /* Background container */
    lv_obj_t *background = lv_obj_create(widget);
    lv_obj_remove_style_all(background);
    lv_obj_set_size(background, width, height);
    // Make the background invisible for now
    lv_obj_set_style_opa(background, LV_OPA_TRANSP, 0);

    /* Background circles */
    lv_obj_t *circles[FRI3D_LOGO_CIRCLES];

    for (int i = 0; i < FRI3D_LOGO_CIRCLES; i++)
    {
        circles[i] = create_circle(background, square_xy / 2);
    }

    /* Fill for the logo */
    lv_obj_t *fill_logo = lv_canvas_create(background);
    lv_obj_set_size(fill_logo, square_xy, square_xy);
    lv_obj_center(fill_logo);

    lv_draw_buf_t *buffer = lv_draw_buf_create(square_xy, square_xy, LV_COLOR_FORMAT_ARGB8888, 0);
    // Make sure the buffer gets cleaned up
    lv_obj_add_event_cb(fill_logo, fri3d_clear_buffer, LV_EVENT_DELETE, buffer);
    lv_canvas_set_draw_buf(fill_logo, buffer);

    lv_canvas_fill_bg(fill_logo, lv_color_hex3(0xFFF), LV_OPA_TRANSP);

    lv_layer_t layer;
    lv_canvas_init_layer(fill_logo, &layer);

    lv_draw_triangle_dsc_t triangle_right;
    lv_draw_triangle_dsc_init(&triangle_right);
    triangle_right.bg_color = background_color;
    triangle_right.p[0].x = lv_pct_to_px(FRI3D_LOGO_TOP_RIGHT_X, square_xy);
    triangle_right.p[0].y = lv_pct_to_px(FRI3D_LOGO_TOP_RIGHT_Y, square_xy);
    triangle_right.p[1].x = lv_pct_to_px(FRI3D_LOGO_MIDDLE_LEFT_X, square_xy);
    triangle_right.p[1].y = lv_pct_to_px(FRI3D_LOGO_MIDDLE_LEFT_Y, square_xy);
    triangle_right.p[2].x = lv_pct_to_px(FRI3D_LOGO_BOTTOM_X, square_xy);
    triangle_right.p[2].y = lv_pct_to_px(FRI3D_LOGO_BOTTOM_Y, square_xy);
    lv_draw_triangle(&layer, &triangle_right);

    lv_draw_triangle_dsc_t triangle_left;
    lv_draw_triangle_dsc_init(&triangle_left);
    triangle_left.bg_color = background_color;
    triangle_left.p[0].x = lv_pct_to_px(FRI3D_LOGO_TOP_LEFT_X, square_xy);
    triangle_left.p[0].y = lv_pct_to_px(FRI3D_LOGO_TOP_LEFT_Y, square_xy);
    triangle_left.p[1].x = lv_pct_to_px(FRI3D_LOGO_MIDDLE_RIGHT_X, square_xy);
    triangle_left.p[1].y = lv_pct_to_px(FRI3D_LOGO_MIDDLE_RIGHT_Y, square_xy);
    triangle_left.p[2].x = lv_pct_to_px(FRI3D_LOGO_BOTTOM_X, square_xy);
    triangle_left.p[2].y = lv_pct_to_px(FRI3D_LOGO_BOTTOM_Y, square_xy);
    lv_draw_triangle(&layer, &triangle_left);

    lv_canvas_finish_layer(fill_logo, &layer);

    /* Logo Container */
    lv_obj_t *logo = lv_obj_create(widget);
    lv_obj_remove_style_all(logo);
    lv_obj_set_size(logo, square_xy, square_xy);
    lv_obj_center(logo);

    /* Draw lines */
    lv_obj_t *lines = lv_obj_create(logo);
    lv_obj_remove_style_all(lines);
    // Make the lines container slightly smaller than the background canvas, this way we can use the same points for
    // the background.
    lv_obj_set_size(lines, LV_PCT(90), LV_PCT(90));
    lv_obj_center(lines);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_line_width(&style, 3);
    lv_style_set_line_color(&style, lv_color_white());

    static lv_style_t cross_background_style;
    lv_style_init(&cross_background_style);
    lv_style_set_line_width(&cross_background_style, 10);
    lv_style_set_line_color(&cross_background_style, background_color);

    uint32_t delay = 0;

    // Lines up
    static lv_point_precise_t points_up_r[] = {FRI3D_LOGO_BOTTOM, FRI3D_LOGO_TOP_RIGHT};
    static lv_point_precise_t points_up_l[] = {FRI3D_LOGO_BOTTOM, FRI3D_LOGO_TOP_LEFT};

    fri3d_lv_animated_line_config_t config_up = {
        .style = &style,
        .points = points_up_r,
        .duration = 800,
        .delay = delay,
        .path_cb = lv_anim_path_ease_in,
    };
    delay += config_up.duration;

    fri3d_lv_animated_line(lines, config_up);
    config_up.points = points_up_l;
    fri3d_lv_animated_line(lines, config_up);

    // To create the break on cross, we draw a wide line in the background color, with on top a regular line
    // Order of line creation is important here!

    // Background cross line, only starts halfway, so it won't interfere with the lines going up
    static lv_point_precise_t points_cross_background_l[] = {FRI3D_LOGO_CROSS_LEFT, FRI3D_LOGO_MIDDLE_LEFT};
    static lv_point_precise_t points_cross_background_r[] = {FRI3D_LOGO_CROSS_RIGHT, FRI3D_LOGO_MIDDLE_RIGHT};

    fri3d_lv_animated_line_config_t config_cross_background = {
        .style = &cross_background_style,
        .points = points_cross_background_l,
        .duration = 250,
        .delay = delay + 250,
        .path_cb = lv_anim_path_linear,
    };

    fri3d_lv_animated_line(lines, config_cross_background);
    config_cross_background.points = points_cross_background_r;
    fri3d_lv_animated_line(lines, config_cross_background);

    // Foreground cross line
    static lv_point_precise_t points_cross_l[] = {FRI3D_LOGO_TOP_RIGHT, FRI3D_LOGO_MIDDLE_LEFT};
    static lv_point_precise_t points_cross_r[] = {FRI3D_LOGO_TOP_LEFT, FRI3D_LOGO_MIDDLE_RIGHT};

    fri3d_lv_animated_line_config_t config_cross = {
        .style = &style,
        .points = points_cross_l,
        .duration = 500,
        .delay = delay,
        .path_cb = lv_anim_path_linear,
    };
    delay += config_cross.duration;

    fri3d_lv_animated_line(lines, config_cross);
    config_cross.points = points_cross_r;
    fri3d_lv_animated_line(lines, config_cross);

    // Lines going down
    static lv_point_precise_t points_down_l[] = {FRI3D_LOGO_MIDDLE_LEFT, FRI3D_LOGO_BOTTOM};
    static lv_point_precise_t points_down_r[] = {FRI3D_LOGO_MIDDLE_RIGHT, FRI3D_LOGO_BOTTOM};

    fri3d_lv_animated_line_config_t config_down = {
        .style = &style,
        .points = points_down_l,
        .duration = 800,
        .delay = delay,
        .path_cb = lv_anim_path_ease_out,
    };
    delay += config_down.duration;

    fri3d_lv_animated_line(lines, config_down);
    config_down.points = points_down_r;
    fri3d_lv_animated_line(lines, config_down);

    /* Animations */
    lv_anim_t a_background;
    lv_anim_init(&a_background);
    lv_anim_set_var(&a_background, background);
    lv_anim_set_values(&a_background, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&a_background, 1);
    lv_anim_set_delay(&a_background, delay);
    lv_anim_set_path_cb(&a_background, lv_anim_path_linear);
    lv_anim_set_exec_cb(&a_background, animate_opa);

    lv_anim_start(&a_background);

    for (int i = 0; i < FRI3D_LOGO_CIRCLES; i++)
    {
        lv_anim_t a_scale;
        lv_anim_init(&a_scale);
        lv_anim_set_var(&a_scale, circles[i]);
        lv_anim_set_values(
            &a_scale,
            square_xy / 2,
            // Divides the circles equally between center point and corner
            square_xy / 2 + (((diag - (square_xy / 2)) / (FRI3D_LOGO_CIRCLES + 1)) * (FRI3D_LOGO_CIRCLES - i)));
        lv_anim_set_duration(&a_scale, 900);
        lv_anim_set_delay(&a_scale, delay);
        lv_anim_set_path_cb(&a_scale, lv_anim_path_linear);
        lv_anim_set_exec_cb(&a_scale, animate_circle);

        lv_anim_start(&a_scale);

        delay += 300;
    }

    return widget;
}
