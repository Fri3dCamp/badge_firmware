include(${CMAKE_BINARY_DIR}/../../../cmake/fri3d_component.cmake)

set(SRCS
        "src/bsp_button.c"
        "src/bsp_buzzer.c"
        "src/bsp_display.c"
        "src/bsp_led.c"
        "src/button_custom.c"
        "src/led_blink_defaults.c"
)

if (CONFIG_FRI3D_ADC)
    list(APPEND SRCS
        "src/bsp_adc.c"
        "src/adc_driver/adc_driver.c"
    )
endif ()

if (CONFIG_FRI3D_JOYSTICK)
    list(APPEND SRCS
        "src/bsp_joystick.c"
        "src/joystick_axis/joystick_axis.c"
    )
endif ()

if (CONFIG_FRI3D_BADGE_FOX)
    list(APPEND SRCS
            "src/boards/fox/bsp_adc.c"
            "src/boards/fox/bsp_button.c"
            "src/boards/fox/bsp_joystick.c"
    )

elseif (CONFIG_FRI3D_BADGE_OCTOPUS)
    list(APPEND SRCS
            "src/boards/octopus/bsp_button.c"
    )

endif ()

set(DEPS
        "driver"
        "esp_adc"
        "esp_lcd"
)

set(PRIV_DEPS
        ""
)

idf_component_register(
        SRCS
        "${SRCS}"
        INCLUDE_DIRS
        "include"
        PRIV_INCLUDE_DIRS
        "src/include"
        REQUIRES
        "${DEPS}"
        PRIV_REQUIRES
        "${PRIV_DEPS}"
)

fri3d_set_loglevel()
