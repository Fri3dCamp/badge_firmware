#include "led_indicator.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Default LED effects */
enum
{
    BSP_LED_ON,
    BSP_LED_OFF,
    BSP_LED_BLINK_FAST,
    BSP_LED_BLINK_SLOW,
    BSP_LED_BREATHE_FAST,
    BSP_LED_BREATHE_SLOW,
    BSP_LED_BLINK_FLOWING,
    BSP_LED_MAX,
};

// Leds
/**
 * @brief Initialize all LEDs
 *
 * @note `led_cnt` and `led_array_size` unused, only one config needed to control the leds
 *
 * @param[out] led_array      Output LED array
 * @param[out] led_cnt        Number of LED handlers saved to led_array, can be NULL
 * @param[in]  led_array_size Size of output LED array. Must be at least BSP_LED_NUM
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t bsp_led_indicator_create(led_indicator_handle_t led_array[], int *led_cnt, int led_array_size);

#ifdef __cplusplus
}
#endif
