#pragma once

#include "esp_lcd_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BSP display configuration structure
 *
 */
typedef struct
{
    int max_transfer_sz;    /*!< Maximum transfer size, in bytes. */
} bsp_display_config_t;

/**
 * @brief Create new display panel
 *
 * For maximum flexibility, this function performs only reset and initialization of the display.
 * You must turn on the display explicitly by calling esp_lcd_panel_disp_on_off().
 * The display's backlight is not turned on either. You can use bsp_display_backlight_on/off(),
 * bsp_display_brightness_set() (on supported boards) or implement your own backlight control.
 *
 * If you want to free resources allocated by this function, you can use esp_lcd API, ie.:
 *
 * \code{.c}
 * esp_lcd_panel_del(panel);
 * esp_lcd_panel_io_del(io);
 * spi_bus_free(spi_num_from_configuration);
 * \endcode
 *
 * @param[in]  config    display configuration
 * @param[out] ret_panel esp_lcd panel handle
 * @param[out] ret_io    esp_lcd IO handle
 * @return
 *      - ESP_OK         On success
 *      - Else           esp_lcd failure
 */
esp_err_t bsp_display_new(
    const bsp_display_config_t *config,
    esp_lcd_panel_handle_t *ret_panel,
    esp_lcd_panel_io_handle_t *ret_io
);

/**
 * @brief Fill the entire screen with a certain color
 *
 * Note that this is a very crude implementation intended for initialization / testing. It will also create a temporary
 * buffer to draw into. If you are going to do extensive drawing, use a proper library like LVGL for it.
 *
 * @param panel the esp_lcd panel to draw on
 * @param color the color to draw with
 * @return
 */
esp_err_t bsp_display_fill(esp_lcd_panel_handle_t panel, uint16_t color);

#ifdef __cplusplus
}
#endif
