#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_FRI3D_BUZZER
/**
 * @brief Deinitialize the LEDC timer associated with the buzzer
 *
 */
void buzzer_deinit();

/**
 * @brief Play a sound on the buzzer with Frequency freq for a duration (ms)
 *
 * @param freq the frequency of the sound
 * @param duration the duration (ms)
 */
void buzzer_tone(uint32_t freq, uint32_t duration, uint8_t volume);

#endif

#ifdef __cplusplus
}
#endif
