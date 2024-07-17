#include "esp_lcd_panel_ops.h"
#include "esp_log.h"

#include "fri3d_application/app_manager.hpp"
#include "fri3d_application/application.hpp"
#include "fri3d_bsp/bsp.h"
#include "fri3d_launcher/launcher.hpp"

using namespace Fri3d::Application;
using namespace Fri3d::Apps;

extern "C" {
static const char *TAG = "main";

#if BSP_CAPS_LED
static led_indicator_handle_t leds[1];
#endif

#if BSP_CAPS_BUTTONS
static void btn_handler(void *button_handle, void *usr_data)
{
    int button_pressed = (int)usr_data;
    ESP_LOGI(TAG, "button pressed %d", button_pressed);
}
#endif

void app_main(void)
{
    esp_log_level_set(TAG, LOG_LOCAL_LEVEL);

#if BSP_CAPS_LED
    ESP_ERROR_CHECK(bsp_led_indicator_create(leds, nullptr, 1));
    led_indicator_start(leds[0], BSP_LED_BLINK_FLOWING);
#endif

#if BSP_CAPS_BUTTONS
    /* Init fri3d buttons */
    ESP_LOGI(TAG, "Init fri3d buttons: %d", BSP_BUTTON_NUM);
    button_handle_t btns[BSP_BUTTON_NUM];
    ESP_ERROR_CHECK(bsp_iot_button_create(btns, nullptr, BSP_BUTTON_NUM));
    for (int i = 0; i < BSP_BUTTON_NUM; i++)
    {
        ESP_ERROR_CHECK(iot_button_register_cb(btns[i], BUTTON_PRESS_DOWN, btn_handler, (void *)i));
    }
#endif

#if BSP_CAPS_BUZZER
    ESP_LOGI(TAG, "Init buzzer");
    buzzer_tone(440, 200);
    buzzer_tone(494, 200);
    buzzer_tone(523, 200);
    buzzer_tone(440, 300);

    buzzer_deinit();
#endif

#if BSP_CAPS_DISPLAY
    ESP_LOGI(TAG, "Init display");
    esp_lcd_panel_io_handle_t io_handle = nullptr;
    esp_lcd_panel_handle_t panel_handle = nullptr;

    const bsp_display_config_t bsp_disp_cfg = {
        .max_transfer_sz = BSP_LCD_WIDTH * sizeof(uint16_t),
    };

    ESP_ERROR_CHECK(bsp_display_new(&bsp_disp_cfg, &panel_handle, &io_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_ERROR_CHECK(bsp_display_fill(panel_handle, 0xFF00));

#endif
    application.init();

    auto &appManager = application.getAppManager();

    appManager.registerApp(Launcher::launcher);

    application.run(Launcher::launcher);

    application.deinit();
}
};
