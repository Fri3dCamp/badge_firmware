#include <stdio.h>
#include "fri3d_bsp/bsp.h"

#if BSP_CAPS_LED
static led_indicator_handle_t leds[1];
#endif

void app_main(void)
{
#if BSP_CAPS_LED
    ESP_ERROR_CHECK(bsp_led_indicator_create(leds, NULL, 1));
    led_indicator_start(leds[0], BSP_LED_BLINK_FLOWING);
#endif
}
