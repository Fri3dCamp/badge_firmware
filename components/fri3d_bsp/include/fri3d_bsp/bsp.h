#pragma once

#include "sdkconfig.h"

#ifdef CONFIG_FRI3D_BADGE_FOX
#include "boards/fox/bsp.h"
#elifdef CONFIG_FRI3D_BADGE_OCTOPUS
#include "boards/octopus/bsp.h"
#else
#error "Unknown board type"
#endif

#include "bsp_led.h"
