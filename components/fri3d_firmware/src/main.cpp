#include "esp_lcd_panel_ops.h"
#include "esp_log.h"

#include "fri3d_application/application.hpp"
#include "fri3d_bsp/bsp.h"
#include "fri3d_launcher/launcher.hpp"
#include "fri3d_splash/splash.hpp"

using namespace Fri3d::Application;
using namespace Fri3d::Apps;

extern "C" {
static const char *TAG = "main";

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

    application.init();

    auto &appManager = application.getAppManager();

    appManager.registerApp(Launcher::launcher);
    appManager.registerApp(Splash::splash);

    application.run(Launcher::launcher);

    application.deinit();
}
};
