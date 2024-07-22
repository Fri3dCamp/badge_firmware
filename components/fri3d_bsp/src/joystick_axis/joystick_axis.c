#include "esp_log.h"

#include "joystick_axis/joystick_axis.h"

static const char *TAG = "joystick_axis";

#define JST_CHECK(a, str, ret_val)                                                                                     \
    if (!(a))                                                                                                          \
    {                                                                                                                  \
        ESP_LOGE(TAG, "%s(%d): %s", __FUNCTION__, __LINE__, str);                                                      \
        return (ret_val);                                                                                              \
    }

#define JST_DEINIT_CHECK(a, str)                                                                                       \
    if (!(a))                                                                                                          \
    {                                                                                                                  \
        ESP_LOGE(TAG, "%s(%d): %s", __FUNCTION__, __LINE__, str);                                                      \
        joystick_axis_deinit_handle(axis);                                                                             \
        return NULL;                                                                                                   \
    }

typedef struct
{
    adc_driver_handle_t adc;
    adc_driver_channel_t channel;
    uint16_t min;
    uint16_t max;
    uint16_t dead_val;
    // We sacrifice some bytes for faster calculations
    uint16_t dead_min;
    uint16_t dead_max;
    float factor;
} joystick_axis;

static esp_err_t joystick_axis_deinit_handle(joystick_axis *axis)
{
    if (axis != NULL)
    {
        free(axis);
    }

    return ESP_OK;
}

static joystick_axis *joystick_axis_init_handle(const joystick_axis_config_t *config)
{
    joystick_axis *axis = calloc(1, sizeof(joystick_axis));
    JST_DEINIT_CHECK(NULL != axis, "Could not alloc memory for axis");

    return axis;
}

void joystick_axis_recalibrate(joystick_axis *axis)
{
    uint16_t range = ((axis->max - axis->min) / 2) - axis->dead_val;

    axis->dead_min = axis->min + range;
    axis->dead_max = axis->max - range;
    axis->factor = 100 / (float)range;

    ESP_LOGD(
        TAG,
        "Recalibrated; Min: %d; Max: %d; Dead min: %d; Dead max: %d; Factor : %f",
        axis->min,
        axis->max,
        axis->dead_min,
        axis->dead_max,
        axis->factor);
}

joystick_axis_handle_t joystick_axis_create(adc_driver_handle_t adc, const joystick_axis_config_t *config)
{
    esp_log_level_set(TAG, LOG_LOCAL_LEVEL);

    JST_CHECK(NULL != config, "Invalid configuration", NULL);

    ESP_LOGD(TAG, "Joystick Axis on ADC channel %hhu", config->adc_channel);

    joystick_axis *axis = joystick_axis_init_handle(config);
    if (axis == NULL)
    {
        return NULL;
    }

    JST_DEINIT_CHECK(NULL != adc, "Invalid ADC driver handle");
    axis->adc = adc;
    axis->channel = config->adc_channel;

    JST_DEINIT_CHECK(config->min < config->max, "Range maximum is smaller than minimum");
    JST_DEINIT_CHECK(config->dead_val * 2 < config->max - config->min, "Dead value is larger than range");
    axis->min = config->min;
    axis->max = config->max;
    axis->dead_val = config->dead_val;

    joystick_axis_recalibrate(axis);

    return axis;
}

esp_err_t joystick_axis_delete(joystick_axis_handle_t handle)
{
    joystick_axis *axis = (joystick_axis *)handle;

    return joystick_axis_deinit_handle(axis);
}

esp_err_t joystick_axis_read(joystick_axis_handle_t handle, int8_t *value)
{
    joystick_axis *axis = (joystick_axis *)handle;

    int raw;

    JST_CHECK(
        ESP_OK == adc_driver_read_channel(axis->adc, axis->channel, &raw),
        "Could not read value from ADC",
        ESP_FAIL);

    JST_CHECK(raw <= UINT16_MAX, "Invalid voltage", ESP_FAIL);
    uint16_t voltage = raw;

    if (voltage < axis->min)
    {
        axis->min = voltage;
        joystick_axis_recalibrate(axis);
    }
    else if (voltage > axis->max)
    {
        axis->max = voltage;
        joystick_axis_recalibrate(axis);
    }

    if (axis->dead_min <= voltage && voltage <= axis->dead_max)
    {
        *value = 0;
    }
    else if (voltage < axis->dead_min)
    {
        *value = 0 - (int8_t)((axis->dead_min - voltage) * axis->factor);
    }
    else if (voltage > axis->dead_max)
    {
        *value = (int8_t)((voltage - axis->dead_max) * axis->factor);
    }

    ESP_LOGV(TAG, "ADC Channel: %d; Value: %d", axis->channel, *value);

    return ESP_OK;
}
