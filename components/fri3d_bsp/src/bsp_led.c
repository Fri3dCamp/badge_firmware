#include "fri3d_bsp/bsp.h"
#include "led_indicator.h"

extern blink_step_t const *bsp_led_blink_defaults_lists[];

// Configuration of the LED Strip
static const led_strip_config_t bsp_leds_rgb_strip_config = {
    .strip_gpio_num = BSP_LED_RGB_GPIO,
    .max_leds = BSP_LED_NUM,
    .led_pixel_format = LED_PIXEL_FORMAT_GRB,
    .led_model = LED_MODEL_WS2812,
    .flags.invert_out = false,
};

// Configuration of RMT
static const led_strip_rmt_config_t bsp_leds_rgb_rmt_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
    .resolution_hz = 10 * 1000 * 1000,     // RMT counter clock frequency = 10MHz
    .flags.with_dma = false,               // DMA feature is available on ESP target like ESP32-S3
};

// Configuration of strips for led_indicator
static led_indicator_strips_config_t bsp_leds_rgb_config = {
    .led_strip_cfg = bsp_leds_rgb_strip_config,
    .led_strip_driver = LED_STRIP_RMT,
    .led_strip_rmt_cfg = bsp_leds_rgb_rmt_config,
};

// Configuration of led_indicator
static const led_indicator_config_t bsp_leds_config = {
    .mode = LED_STRIPS_MODE,
    .led_indicator_strips_config = &bsp_leds_rgb_config,
    .blink_lists = bsp_led_blink_defaults_lists,
    .blink_list_num = BSP_LED_MAX,
};

esp_err_t bsp_led_indicator_create(led_indicator_handle_t led_array[], int *led_cnt, int led_array_size)
{
    if (led_array == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    led_array[0] = led_indicator_create(&bsp_leds_config);
    if (led_array[0] == NULL) {
        return ESP_FAIL;
    }
    return ESP_OK;
}
