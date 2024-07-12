set(SRCS
        "src/launcher.c"
)

set(DEPS
        ""
)

set(PRIV_DEPS
        "fri3d_application"
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
