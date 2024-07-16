#pragma once

#include <stdint.h>
#include <string.h>

#include "esp_err.h"

#include "fri3d_bsp/bsp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t duration; // Allowed values: 1, 2, 4, 8, 16, 32
    uint8_t octave;   // Allowed values: 4, 5, 6, 7
    uint16_t bpm;     // Allowed values: 25, 28, 31, 35, 40, 45, 50, 56, 63, 70, 80, 90, 100, 112, 125, 140, 160, 180,
                      // 200, 225, 250, 285, 320, 355, 400, 450, 500, 565, 635, 715, 800 and 900.
    float msec_whole_note;
} rtttl_default_values_t;

/**
 * @brief play rtttl song on the buzzer
 *
 * @param song the song to play in rtttl format
 * @return esp_err_t
 */
esp_err_t play_rtttl(const char *song, uint8_t volume);

/**
 * @brief play rtttl song on the buzzer in separate task
 *
 * @param song the song to play in rtttl format
 */
void play_rtttl_task(const char *song, uint8_t volume);

#ifdef __cplusplus
}
#endif
