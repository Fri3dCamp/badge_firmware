#include <stdio.h>

#include "fri3d_bsp/bsp.h"
#include "fri3d_application/application.h"
#include "fri3d_launcher/launcher.h"

#if BSP_CAPS_LED
static led_indicator_handle_t leds[1];
#endif

void app_main(void)
{
#if BSP_CAPS_LED
    ESP_ERROR_CHECK(bsp_led_indicator_create(leds, NULL, 1));
    led_indicator_start(leds[0], BSP_LED_BLINK_FLOWING);
#endif
    fri3d_application_create();
    fri3d_launcher_create();
}
