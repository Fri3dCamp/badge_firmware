set(SRCS
        "src/bsp_led.c"
        "src/led_blink_defaults.c"
)

set(DEPS
        "driver"
)

set(PRIV_DEPS
        ""
)

idf_component_register(
        SRCS
        "${SRCS}"
        INCLUDE_DIRS
        "include"
        REQUIRES
        "${DEPS}"
        PRIV_REQUIRES
        "${PRIV_DEPS}"
)
