#include "esp_log.h"
#include "iot_button.h"

#include "fri3d_bsp/bsp.h"

#include "fri3d_private/button_custom.h"

static const char *TAG = "fri3d_bsp_button";

extern const bsp_button_config_t bsp_button_config[BSP_BUTTON_NUM];

esp_err_t bsp_iot_button_create(button_handle_t btn_array[], int *btn_cnt, int btn_array_size)
{
    esp_log_level_set(TAG, LOG_LOCAL_LEVEL);

    esp_err_t ret = ESP_OK;
    if ((btn_array_size < BSP_BUTTON_NUM) || (btn_array == NULL))
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (btn_cnt)
    {
        *btn_cnt = 0;
    }
    for (int i = 0; i < BSP_BUTTON_NUM; i++)
    {
        button_config_t *config = (button_config_t *)calloc(1, sizeof(button_config_t));

        if (config == NULL)
        {
            ESP_LOGE(TAG, "Memory error calloc button_config_t failed.");
            ret = ESP_FAIL;
            break;
        }

        if (bsp_button_config[i].gpio_pull_mode == GPIO_PULLUP_ONLY)
        {
            config->type = BUTTON_TYPE_GPIO;
            config->gpio_button_config.gpio_num = bsp_button_config[i].gpio_num;
            config->gpio_button_config.active_level = 0;
        }

        else if (bsp_button_config[i].gpio_pull_mode == GPIO_FLOATING)
        {
            config->type = BUTTON_TYPE_CUSTOM;
            config->custom_button_config.active_level = 0;
            config->custom_button_config.button_custom_init = bsp_button_custom_init;
            config->custom_button_config.button_custom_get_key_value = bsp_button_custom_get_key_level;
            config->custom_button_config.button_custom_deinit = NULL;

            bsp_button_custom_config_t *custom_private =
                (bsp_button_custom_config_t *)calloc(1, sizeof(bsp_button_custom_config_t));
            if (custom_private == NULL)
            {
                ESP_LOGE(TAG, "Memory error calloc bsp_button_custom_config_t failed.");
                ret = ESP_FAIL;
                break;
            }
            custom_private->gpio_num = bsp_button_config[i].gpio_num;
            custom_private->active_level = 0;
            custom_private->gpio_pull_mode = bsp_button_config[i].gpio_pull_mode;
#if CONFIG_GPIO_BUTTON_SUPPORT_POWER_SAVE
            custom_private->enable_power_save = true;
#endif

            config->custom_button_config.priv = custom_private;
        }
        else
        {
            ESP_LOGE(TAG, "Unsupported gpio_pull_mode: %d", bsp_button_config[i].gpio_pull_mode);
            ret = ESP_FAIL;
            break;
        }

        btn_array[i] = iot_button_create(config);
        if (btn_array[i] == NULL)
        {
            ret = ESP_FAIL;
            break;
        }
        if (btn_cnt)
        {
            (*btn_cnt)++;
        }
    }
    return ret;
};
