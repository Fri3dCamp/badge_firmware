#include "esp_lcd_panel_ops.h"
#include "esp_log.h"

#include "fri3d_application/application.hpp"
#include "fri3d_launcher/launcher.hpp"
#include "fri3d_splash/splash.hpp"

using namespace Fri3d::Application;
using namespace Fri3d::Apps;

extern "C" {
static const char *TAG = "main";

void app_main(void)
{
    esp_log_level_set(TAG, LOG_LOCAL_LEVEL);

    application.init();

    auto &appManager = application.getAppManager();

    appManager.registerApp(Launcher::launcher);
    appManager.registerApp(Splash::splash);

    application.run(Launcher::launcher);

    application.deinit();
}
};
