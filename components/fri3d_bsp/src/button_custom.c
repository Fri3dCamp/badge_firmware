
#include "esp_log.h"
#include "fri3d_bsp/button_custom.h"
#include "driver/gpio.h"

static const char *TAG = "button custom";

#define GPIO_BTN_CHECK(a, str, ret_val)                           \
    if (!(a))                                                     \
    {                                                             \
        ESP_LOGE(TAG, "%s(%d): %s", __FUNCTION__, __LINE__, str); \
        return (ret_val);                                         \
    }

esp_err_t bsp_button_custom_init(void *param)
{
    const bsp_button_custom_config_t *config = (bsp_button_custom_config_t *) param;

    GPIO_BTN_CHECK(NULL != config, "Pointer of config is invalid", ESP_ERR_INVALID_ARG);
    GPIO_BTN_CHECK(GPIO_IS_VALID_GPIO(config->gpio_num), "GPIO number error", ESP_ERR_INVALID_ARG);

    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pin_bit_mask = (1ULL << config->gpio_num);
    if (config->gpio_pull_mode == GPIO_PULLUP_ONLY)
    {
        gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    }
    else if (config->gpio_pull_mode == GPIO_PULLDOWN_ONLY)
    {
        gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    }
    else if (config->gpio_pull_mode == GPIO_FLOATING)
    {
        gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    }
    else
    {
        ESP_LOGE(TAG, "Button Custom Init unsupported gpio_pull_mode: %d", config->gpio_pull_mode);
    }
    gpio_config(&gpio_conf);

#if CONFIG_GPIO_BUTTON_SUPPORT_POWER_SAVE
    if (config->enable_power_save) {
        /* Enable wake up from GPIO */
        esp_err_t ret = gpio_wakeup_enable(config->gpio_num, config->active_level == 0 ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL);
        GPIO_BTN_CHECK(ret == ESP_OK, "Enable gpio wakeup failed", ESP_FAIL);
        ret = esp_sleep_enable_gpio_wakeup();
        GPIO_BTN_CHECK(ret == ESP_OK, "Configure gpio as wakeup source failed", ESP_FAIL);
    }
#endif

    return ESP_OK;
}

uint8_t bsp_button_custom_get_key_level(void *param)
{
    const bsp_button_custom_config_t *config = (bsp_button_custom_config_t *) param;
    return (uint8_t) gpio_get_level((uint32_t) config->gpio_num);
}
