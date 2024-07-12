set(SRCS
        "src/application.c"
)

set(DEPS
        ""
)

set(PRIV_DEPS
        "fri3d_bsp"
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
