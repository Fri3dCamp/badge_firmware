#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdexcept>

#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "fri3d_application/lvgl.hpp"
#include "fri3d_bsp/bsp.h"
#include "fri3d_private/lvgl.hpp"

#define BUF_SIZE (BSP_LCD_WIDTH * 20)

// Temporary workaround, see public lvgl.hpp for more info
#if LV_USE_OS != LV_OS_NONE
static lv_mutex_t lv_general_mutex;
#endif /*LV_USE_OS != LV_OS_NONE*/

static void lv_os_init(void)
{
#if LV_USE_OS != LV_OS_NONE
    lv_mutex_init(&lv_general_mutex);
#endif /*LV_USE_OS != LV_OS_NONE*/
}

static void lv_os_deinit(void)
{
#if LV_USE_OS != LV_OS_NONE
    lv_mutex_delete(&lv_general_mutex);
#endif /*LV_USE_OS != LV_OS_NONE*/
}

void lv_lock(void)
{
#if LV_USE_OS != LV_OS_NONE
    lv_mutex_lock(&lv_general_mutex);
#endif /*LV_USE_OS != LV_OS_NONE*/
}

lv_result_t lv_lock_isr(void)
{
#if LV_USE_OS != LV_OS_NONE
    return lv_mutex_lock_isr(&lv_general_mutex);
#else  /*LV_USE_OS != LV_OS_NONE*/
    return LV_RESULT_OK;
#endif /*LV_USE_OS != LV_OS_NONE*/
}

void lv_unlock(void)
{
#if LV_USE_OS != LV_OS_NONE
    lv_mutex_unlock(&lv_general_mutex);
#endif /*LV_USE_OS != LV_OS_NONE*/
}

namespace Fri3d::Application
{

static const char *TAG = "Fri3d::Application::CLVGL";

CLVGL::CLVGL()
    : buf1(nullptr)
    , buf2(nullptr)
    , panel(nullptr)
    , panel_io(nullptr)
    , running(false)
{
    esp_log_level_set(TAG, static_cast<esp_log_level_t>(LOG_LOCAL_LEVEL));
}

CLVGL::~CLVGL()
{
    this->deinit();
}

void CLVGL::init()
{
    if (this->panel == nullptr && this->panel_io == nullptr)
    {
        auto config = bsp_display_config_t{
            .max_transfer_sz = 100,
            .on_color_trans_done = CLVGL::on_color_trans_done,
            .user_ctx = this};
        ESP_ERROR_CHECK(bsp_display_new(&config, &this->panel, &this->panel_io));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(this->panel, true));
    }

    if (this->panel == nullptr || this->panel_io == nullptr)
    {
        throw std::runtime_error("Display panel is in an invalid state");
    }

    if (!lv_is_initialized())
    {
        ESP_LOGI(TAG, "Initializing LVGL library");
        lv_os_init();
        lv_init();
    }

    lv_lock();

    ESP_LOGI(TAG, "Initializing LVGL display with size %dx%d", BSP_LCD_WIDTH, BSP_LCD_HEIGHT);
    this->lv_display = lv_display_create(BSP_LCD_WIDTH, BSP_LCD_HEIGHT);

    ESP_LOGI(TAG, "Creating display buffers");
    // We use heap_caps_malloc instead of new because we need to be sure about DMA capability
    this->buf1 = static_cast<lv_color_t *>(heap_caps_malloc(BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA));
    assert(this->buf1);
    this->buf2 = static_cast<lv_color_t *>(heap_caps_malloc(BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA));
    assert(this->buf2);

    // initialize LVGL draw buffers
    lv_display_set_buffers(this->lv_display, this->buf1, this->buf2, BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    ESP_LOGI(TAG, "Registering callback functions");
    lv_display_set_user_data(this->lv_display, this->panel);
    lv_display_set_flush_cb(this->lv_display, CLVGL::flush_cb);
    lv_tick_set_cb(CLVGL::tick_get_cb);

    // TODO: move this to a theme manager
    auto theme = lv_theme_default_init(
        this->lv_display,
        lv_palette_main(LV_PALETTE_GREEN),
        lv_palette_main(LV_PALETTE_GREY),
        true,
        &lv_font_montserrat_12);

    lv_display_set_theme(this->lv_display, theme);

    lv_unlock();

    this->indev.init();
}

void CLVGL::deinit()
{
    if (lv_is_initialized())
    {
        this->indev.deinit();

        lv_deinit();
        lv_os_deinit();
    }
}

void CLVGL::flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *data)
{
    auto panel = static_cast<esp_lcd_panel_handle_t>(lv_display_get_user_data(display));

    // Correct byte order
    lv_draw_sw_rgb565_swap(data, BUF_SIZE);

    // Blit to the screen
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, data));
}

bool CLVGL::on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *data, void *user_ctx)
{
    auto *self = static_cast<CLVGL *>(user_ctx);

    if (self->lv_display != nullptr)
    {
        lv_disp_flush_ready(self->lv_display);
    }

    return false;
}

lv_display_t *CLVGL::get_Display()
{
    return this->lv_display;
}

uint32_t CLVGL::tick_get_cb()
{
    return esp_timer_get_time() / 1000;
}

void CLVGL::start()
{
    std::lock_guard lock(this->workerMutex);
    if (this->worker.joinable())
    {
        throw std::runtime_error("Already running");
    }

    this->running = true;
    this->worker = std::thread(&CLVGL::work, this);
}

void CLVGL::stop()
{
    std::lock_guard lock(this->workerMutex);
    if (this->worker.joinable())
    {
        this->running = false;
        this->worker.join();
    }
}

void CLVGL::work() const
{
    // We raise our priority for smoother drawing
    // TODO: maybe put this together with thread creation in a CThreadManager?
    vTaskPrioritySet(NULL, uxTaskPriorityGet(NULL) + 5);

    while (this->running)
    {
        // In LVGL 9.2 and above, the lock will be taken internally in lv_timer_handler() and should be removed here
        lv_lock();
        auto sleep_time = lv_timer_handler();
        lv_unlock();

        if (sleep_time == LV_NO_TIMER_READY)
        {
            // There is nothing to be done, wait for 40 ms (== 25 fps)
            sleep_time = 40ul;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }
}

} // namespace Fri3d::Application
