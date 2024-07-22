#pragma once

#include "sdkconfig.h"

#ifdef CONFIG_FRI3D_BADGE_FOX
#include "boards/fox/bsp.h"
#elifdef CONFIG_FRI3D_BADGE_OCTOPUS
#include "boards/octopus/bsp.h"
#else
#error "Unknown board type"
#endif

#if BSP_CAPS_ADC
#include "bsp_adc.h"
#endif

#if BSP_CAPS_DISPLAY
#include "bsp_display.h"
#endif

#if BSP_CAPS_BUTTONS
#include "bsp_button.h"
#endif

#if BSP_CAPS_JOYSTICK
#include "bsp_joystick.h"
#endif

#if BSP_CAPS_BUZZER
#include "bsp_buzzer.h"
#endif

#if BSP_CAPS_LED
#include "bsp_led.h"
#endif
