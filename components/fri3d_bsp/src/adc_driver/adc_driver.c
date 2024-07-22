#include <pthread.h>

#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "adc_driver/adc_driver.h"

static const char *TAG = "adc_driver";

#define ADC_CHECK(a, str, ret_val)                                                                                     \
    if (!(a))                                                                                                          \
    {                                                                                                                  \
        ESP_LOGE(TAG, "%s(%d): %s", __FUNCTION__, __LINE__, str);                                                      \
        return (ret_val);                                                                                              \
    }

#define ADC_DEINIT_CHECK(a, str)                                                                                       \
    if (!(a))                                                                                                          \
    {                                                                                                                  \
        ESP_LOGE(TAG, "%s(%d): %s", __FUNCTION__, __LINE__, str);                                                      \
        adc_driver_deinit_handle(adc);                                                                                 \
        return NULL;                                                                                                   \
    }

typedef struct
{
    adc_channel_t channel;
    adc_cali_handle_t calibration;
    adc_driver_calibration_t calibration_type;
    // Calls to calibration api are not thread-safe
    pthread_mutex_t calibration_mutex;
} adc_driver_channel;

typedef struct
{
    adc_oneshot_unit_handle_t unit;
    adc_driver_channel_t channel_count;
    adc_driver_channel *channels;
} adc_driver;

static esp_err_t adc_driver_deinit_handle(adc_driver *adc)
{
    if (adc->channels != NULL)
    {
        for (int i = 0; i < adc->channel_count; i++)
        {
            adc_driver_channel *channel = &adc->channels[i];

            ADC_CHECK(0 == pthread_mutex_destroy(&channel->calibration_mutex), "Could not destroy mutex", ESP_FAIL);

            if (channel->calibration != NULL)
            {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
                if (channel->calibration_type == ADC_DRIVER_CALIBRATION_CURVE)
                {
                    ADC_CHECK(
                        ESP_OK == adc_cali_delete_scheme_curve_fitting(channel->calibration),
                        "Could not remove curve fitting",
                        ESP_FAIL);
                }
#endif
#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
                if (channel->calibration_type == ADC_DRIVER_CALIBRATION_LINE)
                {
                    ADC_CHECK(
                        ESP_OK == adc_cali_delete_line_fitting(channel->calibration),
                        "Could not remove line fitting",
                        ESP_FAIL);
                }
#endif
            }
        }
        free(adc->channels);
    }

    if (adc != NULL)
    {
        if (adc->unit != NULL)
        {
            ADC_CHECK(ESP_OK == adc_oneshot_del_unit(adc->unit), "Could not delete ADC unit", ESP_FAIL);
        }

        free(adc);
    }

    return ESP_OK;
}

static adc_driver *adc_driver_init_handle(const adc_driver_config_t *config)
{
    adc_driver *adc = calloc(1, sizeof(adc_driver));
    ADC_DEINIT_CHECK(NULL != adc, "Could not alloc memory for driver");

    adc->channel_count = config->channel_count;
    adc->channels = calloc(adc->channel_count, sizeof(adc_driver_channel));
    ADC_DEINIT_CHECK(NULL != adc, "Could not alloc memory for channels");

    return adc;
}

adc_driver_handle_t adc_driver_create(const adc_driver_config_t *config)
{
    esp_log_level_set(TAG, LOG_LOCAL_LEVEL);

    ADC_CHECK(NULL != config, "Invalid configuration", NULL);

    adc_driver *adc = adc_driver_init_handle(config);
    if (adc == NULL)
    {
        return NULL;
    }

    adc_oneshot_unit_init_cfg_t unit_config = {.unit_id = config->unit};
    ADC_DEINIT_CHECK(ESP_OK == adc_oneshot_new_unit(&unit_config, &adc->unit), "Could not initialize ADC");

    for (int i = 0; i < config->channel_count; i++)
    {
        adc_driver_channel *channel = &adc->channels[i];

        ADC_DEINIT_CHECK(0 == pthread_mutex_init(&channel->calibration_mutex, NULL), "Could not init mutex");

        adc_unit_t unit;
        ADC_DEINIT_CHECK(
            ESP_OK == adc_oneshot_io_to_channel(config->channels[i].gpio, &unit, &channel->channel),
            "GPIO is not connected to an ADC");
        ADC_DEINIT_CHECK(unit == config->unit, "ADC unit does not match GPIO");

        ESP_LOGD(TAG, "Mapped gpio %lu to ADC channel %d", config->channels[i].gpio, channel->channel);

        adc_oneshot_chan_cfg_t channel_config = {
            .bitwidth = config->channels[i].bitwidth,
            .atten = config->channels[i].atten,
        };

        ADC_DEINIT_CHECK(
            ESP_OK == adc_oneshot_config_channel(adc->unit, channel->channel, &channel_config),
            "Could not initialize ADC channel");

        // Store for later usage in deinit
        channel->calibration_type = config->channels[i].calibration;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        if (config->channels[i].calibration == ADC_DRIVER_CALIBRATION_CURVE)
        {
            ESP_LOGD(TAG, "Setting up curve fitting on channel %d", channel->channel);
            adc_cali_curve_fitting_config_t curve_config = {
                .unit_id = unit_config.unit_id,
                .chan = channel->channel,
                channel_config.atten,
                channel_config.bitwidth,
            };

            ADC_DEINIT_CHECK(
                ESP_OK == adc_cali_create_scheme_curve_fitting(&curve_config, &channel->calibration),
                "Could not setup curve fitting calibration");
        }
#endif
#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        if (config->channels[i].calibration == ADC_DRIVER_CALIBRATION_LINE)
        {
            ESP_LOGD(TAG, "Setting up line fitting on channel %d", channel->channel);
            adc_line_fitting_config_t line_config = {
                .unit_id = unit_config.unit_id,
                .chan = channel->channel,
                channel_config.atten,
                channel_config.bitwidth,
            };

            ADC_DEINIT_CHECK(
                ESP_OK == adc_cali_create_scheme_line_fitting(&line_config, &channel->calibration),
                "Coult not setup line fitting calibration");
        }
#endif
    }
    return adc;
}

esp_err_t adc_driver_delete(adc_driver_handle_t handle)
{
    adc_driver *adc = (adc_driver *)handle;

    return adc_driver_deinit_handle(adc);
}

esp_err_t adc_driver_read_channel(adc_driver_handle_t handle, adc_driver_channel_t channel, int *value)
{
    adc_driver *adc = (adc_driver *)handle;

    ADC_CHECK(channel < adc->channel_count, "Invalid channel number", ESP_FAIL);

    adc_driver_channel *adc_channel = &adc->channels[channel];
    int raw;
    ADC_CHECK(
        ESP_OK == adc_oneshot_read(adc->unit, adc_channel->channel, &raw),
        "Could not read from channel",
        ESP_FAIL);

    int voltage = 0;
    if (adc_channel->calibration != NULL)
    {
        pthread_mutex_lock(&adc_channel->calibration_mutex);
        esp_err_t err = adc_cali_raw_to_voltage(adc_channel->calibration, raw, &voltage);
        pthread_mutex_unlock(&adc_channel->calibration_mutex);

        ADC_CHECK(ESP_OK == err, "Could not convert raw to calibrated voltage", ESP_FAIL);

        *value = voltage;
    }
    else
    {
        *value = raw;
    }
    ESP_LOGV(TAG, "Channel: %d; Raw: %d; Calibrated: %d", adc_channel->channel, raw, voltage);

    return ESP_OK;
}
