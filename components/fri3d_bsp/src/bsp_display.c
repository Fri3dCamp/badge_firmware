#include "esp_check.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"

#include "fri3d_bsp/bsp.h"

static const char *TAG = "fri3d_bsp_display";

esp_err_t bsp_display_new(
    const bsp_display_config_t *config,
    esp_lcd_panel_handle_t *ret_panel,
    esp_lcd_panel_io_handle_t *ret_io
)
{
    esp_log_level_set(TAG, LOG_LOCAL_LEVEL);

    esp_err_t ret = ESP_OK;
    assert(config != NULL && config->max_transfer_sz > 0);

    // TODO: This needs to be fleshed out for SD Card support
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = BSP_SPI_SCK,
        .mosi_io_num = BSP_SPI_MOSI,
        .miso_io_num = BSP_SPI_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = config->max_transfer_sz,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_DC,
        .cs_gpio_num = BSP_LCD_CS,
        .pclk_hz = BSP_SPI_BAUDRATE,
        .lcd_cmd_bits = BSP_LCD_CMD_BITS,
        .lcd_param_bits = BSP_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_GOTO_ON_ERROR(
        esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) BSP_SPI_HOST, &io_config, ret_io),
        err,
        TAG,
        "New panel IO failed"
    );

    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST,
        .color_space = BSP_LCD_RGB_ORDER,
        .bits_per_pixel = BSP_LCD_BPP,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7789(*ret_io, &panel_config, ret_panel), err, TAG, "New panel failed");

    ESP_GOTO_ON_ERROR(esp_lcd_panel_reset(*ret_panel), err, TAG, "Failed to reset panel");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_init(*ret_panel), err, TAG, "Failed to init panel");

    ESP_GOTO_ON_ERROR(esp_lcd_panel_invert_color(*ret_panel, BSP_LCD_INVERT),
                      err,
                      TAG,
                      "Failed to set color inversion");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_swap_xy(*ret_panel, BSP_LCD_SWAP_XY), err, TAG, "Failed to set xy swap");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_mirror(*ret_panel, BSP_LCD_MIRROR_X, BSP_LCD_MIRROR_Y),
                      err,
                      TAG,
                      "Failed to set XY mirroring");

    ret = bsp_display_fill(*ret_panel, 0x0000);
    return ret;

    err:
    if (*ret_panel)
    {
        esp_lcd_panel_del(*ret_panel);
    }
    if (*ret_io)
    {
        esp_lcd_panel_io_del(*ret_io);
    }
    spi_bus_free(BSP_SPI_HOST);
    return ret;
}

esp_err_t bsp_display_fill(esp_lcd_panel_handle_t panel, uint16_t color)
{
    esp_err_t ret = ESP_OK;
    size_t buf_size = BSP_LCD_WIDTH;
    uint16_t *buf = heap_caps_malloc(buf_size * sizeof(uint16_t), MALLOC_CAP_DMA);

    assert(buf);
    for (int x = 0; x < BSP_LCD_WIDTH; x++)
    {
        buf[x] = color;
    }

    for (int line = 0; line < BSP_LCD_HEIGHT; line++)
    {
        ESP_GOTO_ON_ERROR(esp_lcd_panel_draw_bitmap(panel, 0, line, BSP_LCD_WIDTH, line + 1, buf),
                          err,
                          TAG,
                          "Could not draw to panel");
    }

    heap_caps_free(buf);

    return ret;
    err:
    heap_caps_free(buf);
    return ret;
}
