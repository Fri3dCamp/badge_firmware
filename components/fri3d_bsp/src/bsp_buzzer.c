#include "esp_log.h"

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "stdio.h"

#include "fri3d_bsp/bsp.h"

void buzzer_deinit()
{
    // stop
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

    // Also, the timer has to be paused first before it can be de-configured
    ESP_ERROR_CHECK(ledc_timer_pause(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

    ledc_timer_config_t timer_conf;
    timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_conf.timer_num = LEDC_TIMER_0;
    timer_conf.deconfigure = true; // When this field is set, duty_resolution, freq_hz, clk_cfg fields are ignored
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));
}

void buzzer_tone(uint32_t freq, uint32_t duration)
{
    ledc_timer_config_t timer_conf;
    timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_conf.duty_resolution = LEDC_TIMER_10_BIT;
    timer_conf.timer_num = LEDC_TIMER_0;
    timer_conf.freq_hz = freq;
    timer_conf.clk_cfg = LEDC_AUTO_CLK;
    timer_conf.deconfigure = false;
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t ledc_conf;
    ledc_conf.gpio_num = BSP_BUZZER_GPIO;
    ledc_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_conf.channel = LEDC_CHANNEL_0;
    ledc_conf.intr_type = LEDC_INTR_DISABLE;
    ledc_conf.timer_sel = LEDC_TIMER_0;
    ledc_conf.duty = 0x0; // 50%=0x3FFF, 100%=0x7FFF for 15 Bit
    // 50%=0x01FF, 100%=0x03FF for 10 Bit
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_conf));

    // start
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0,
                                  0x7F)); // 12% duty - play here for your speaker or buzzer
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
    vTaskDelay(duration / portTICK_PERIOD_MS);
    // stop
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}
