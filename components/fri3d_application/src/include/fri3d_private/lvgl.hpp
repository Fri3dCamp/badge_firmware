#pragma once

#include <mutex>
#include <thread>

#include "lvgl.h"

#include "fri3d_bsp/bsp_display.h"
#include "fri3d_private/indev.hpp"

namespace Fri3d::Application
{

class CLVGL
{
private:
    CIndev indev;

    lv_color_t *buf1;
    lv_color_t *buf2;

    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t panel_io;
    lv_display_t *lv_display;

    static void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *data);
    static bool on_color_trans_done(
        esp_lcd_panel_io_handle_t panel_io,
        esp_lcd_panel_io_event_data_t *data,
        void *user_ctx);
    static uint32_t tick_get_cb();

    std::thread worker;
    std::mutex workerMutex;
    bool running;
    void work() const;

public:
    CLVGL();
    ~CLVGL();

    void start();
    void stop();

    void init();
    void deinit();

    lv_display_t *get_Display();
};

}; // namespace Fri3d::Application
