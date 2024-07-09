set(SRCS
        "src/main.c"
)

set(PRIV_DEPS
        "fri3d_bsp"
)

idf_component_register(
        SRCS
        "${SRCS}"
        PRIV_REQUIRES
        "${PRIV_DEPS}"
)
