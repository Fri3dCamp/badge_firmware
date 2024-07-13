include(${CMAKE_BINARY_DIR}/../../../cmake/fri3d_component.cmake)

set(SRCS
        "src/bsp_button.c"
        "src/bsp_led.c"
        "src/button_custom.c"
        "src/led_blink_defaults.c"
)

if (CONFIG_FRI3D_BADGE_FOX)
    list(APPEND SRCS
            "src/boards/fox/bsp_button.c"
    )

elseif (CONFIG_FRI3D_BADGE_OCTOPUS)
    list(APPEND SRCS
            "src/boards/octopus/bsp_button.c"
    )

endif ()

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

fri3d_set_loglevel()
