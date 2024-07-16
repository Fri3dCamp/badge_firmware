#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"

#include "fri3d_bsp/bsp.h"
#include "fri3d_util/rtttl/rtttl.h"

static const char *TAG = "rtttl";

/**
 * RTTTL specification
 * https://www.mobilefish.com/tutorials/rtttl/rtttl_quickguide_specification.html
 */

static float NOTES[] = {
    440.0f, // A
    493.9f, // B or H
    261.6f, // C
    293.7f, // D
    329.6f, // E
    349.2f, // F
    392.0f, // G
    0.0f,   // pad

    466.2f, // A#
    0.0f,
    277.2f, // C#
    311.1f, // D#
    0.0f,
    370.0f, // F#
    415.3f, // G#
    0.0f,
};

typedef struct
{
    const char *song;
    uint8_t volume;
} rtttl_task_parameter_t;

/**
 * @brief parse rtttl default values
 *
 * d - Duration (Allowed values: 1, 2, 4, 8, 16, 32)
 * o - Octave (Allowed values: 4, 5, 6, 7)
 * b - Beats per minute (Allowed values: 25, 28, 31, 35, 40, 45, 50, 56, 63, 70, 80, 90, 100, 112, 125, 140, 160, 180,
 * 200, 225, 250, 285, 320, 355, 400, 450, 500, 565, 635, 715, 800 and 900.)
 *
 * @param song_defaults
 * @param defaults_len
 * @param default_values
 * @return esp_err_t
 */
static void parse_defaults(
    const char *song_defaults,
    const uint8_t defaults_len,
    rtttl_default_values_t *default_values)
{
    char delim = ',';
    char *ptr = (char *)song_defaults;
    while (ptr != NULL)
    {
        if (ptr - song_defaults >= defaults_len)
        {
            // reached the end
            break;
        }
        if ((*ptr == 'd' || *ptr == 'o' || *ptr == 'b') && *(ptr + 1) == '=')
        {
            char *end = strchr(ptr, delim);
            if (end == NULL || end > song_defaults + defaults_len)
            {
                end = (char *)song_defaults + defaults_len;
            }

            char *s = ptr + 2;
            uint16_t parsed_int = 0;
            while (isdigit((int)*s))
            {
                parsed_int *= 10;
                parsed_int += (int)*s - (int)'0';
                s++;
                if (*s >= *end)
                {
                    break;
                }
            }

            if (*ptr == 'd')
            {
                default_values->duration = parsed_int;
            }
            else if (*ptr == 'o')
            {
                default_values->octave = parsed_int;
            }
            else if (*ptr == 'b')
            {
                default_values->bpm = parsed_int;

                // 240000 = 60 sec/min * 4 beats/whole-note * 1000 msec/sec
                default_values->msec_whole_note = 240000.0f / (float)parsed_int;
            }

            ptr = end + 1;
        }
        else
        {
            // unuseful character encountered
            ptr++;
        }
    }
}

void parse_single_data(
    const char *single_data,
    const rtttl_default_values_t *default_values,
    uint32_t *freq,
    uint32_t *msec)
{
    char *s = (char *)single_data;

    while (*s == ' ')
    {
        s++;
    }

    // Parse duration, if present. A duration of 1 means a whole note.
    //  A duration of 8 means 1/8 note.
    uint8_t duration = 0;
    while (isdigit((int)*s))
    {
        duration *= 10;
        duration += (int)*s - (int)'0';
        s++;
    }
    if (duration == 0)
    {
        duration = default_values->duration;
    }

    uint8_t note_index;
    char note = tolower(*s);
    if ('a' <= note && note <= 'g')
    {
        note_index = (int)note - (int)'a';
    }
    else if (note == 'h')
    {
        note_index = 1; // h is equivalent to b
    }
    else
    {
        note_index = 7; // pause
    }
    s++;

    // Check for sharp note
    if (*s == '#')
    {
        note_index += 8;
        s++;
    }

    // Check for duration modifier before octave
    // The spec has the dot after the octave, but some places do it
    // the other way around.
    float duration_multiplier = 1.0f;
    if (*s == '.')
    {
        duration_multiplier = 1.5f;
        s++;
    }

    // Check for octave
    uint8_t octave;
    if ('4' <= *s && *s <= '7')
    {
        octave = (int)*s - (int)'0';
        s++;
    }
    else
    {
        octave = default_values->octave;
    }

    // Check for duration modifier after octave
    if (*s == '.')
    {
        duration_multiplier = 1.5f;
        s++;
    }

    ESP_LOGD(
        TAG,
        "duration %d, note_index: %d, octave: %d, duration_multiplier: %4.2f",
        duration,
        note_index,
        octave,
        duration_multiplier);

    *freq = (uint32_t)NOTES[note_index] * (1 << (octave - 4));
    *msec = (uint32_t)((default_values->msec_whole_note / (float)duration) * duration_multiplier);

    ESP_LOGD(TAG, "freq: %" PRIu32 ", msec: %" PRIu32 "", *freq, *msec);
}

// Task to be created.
void vTaskCode(void *pvParameters)
{
    rtttl_task_parameter_t *task_parameter = (rtttl_task_parameter_t *)pvParameters;
    play_rtttl(task_parameter->song, task_parameter->volume);
    vTaskDelete(NULL);
}

// Function that creates a task.
void play_rtttl_task(const char *song, uint8_t volume)
{
    // 10ms seems enough
    vTaskDelay(10 / portTICK_PERIOD_MS);
    static rtttl_task_parameter_t task_parameter;
    task_parameter.song = song;
    task_parameter.volume = volume;

    TaskHandle_t xHandle = NULL;
    xTaskCreatePinnedToCore(
        vTaskCode,
        "play_rtttl",
        2048,
        (void *const)&task_parameter,
        tskIDLE_PRIORITY + 1,
        &xHandle,
        0);
    configASSERT(xHandle);

    // 10ms seems enough
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

esp_err_t play_rtttl(const char *song, uint8_t volume)
{
    esp_err_t ret = ESP_OK;

    char delim = ':';
    uint8_t part_count = 0;
    char *parts[3];
    char *ptr = (char *)song;
    while (ptr != NULL)
    {
        if (part_count >= 3)
        {
            ESP_LOGE(TAG, "only expect 3 parts in song separated by ':'");
            ret = ESP_FAIL;
            return ret;
        }
        parts[part_count] = ptr;
        part_count++;
        ptr = strchr(ptr + 1, delim);
    };

    // display name
    size_t name_len = parts[1] - parts[0];
    ESP_LOGI(TAG, "Now playing '%.*s'.", name_len, parts[0]);

    // parse default values
    size_t defaults_len = parts[2] - parts[1] - 1;
    rtttl_default_values_t default_values;
    parse_defaults(parts[1] + 1, defaults_len, &default_values);

    // parse and play data
    delim = ',';
    ptr = parts[2] + 1;
    while (*ptr != 0)
    {
        char *end = strchr(ptr, delim);
        if (end == NULL)
        {
            end = ptr + strlen(ptr);
        }

        uint32_t freq;
        uint32_t msec;
        parse_single_data(ptr, &default_values, &freq, &msec);

        // play the note
        buzzer_tone(freq, msec, volume);

        ptr = end + 1;
    }

    buzzer_deinit();

    ESP_LOGI(TAG, "Finished playing '%.*s'.", name_len, parts[0]);

    return ret;
}
