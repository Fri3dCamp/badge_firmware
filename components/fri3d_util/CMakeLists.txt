include(${CMAKE_BINARY_DIR}/../../../cmake/fri3d_component.cmake)

set(SRCS
        "src/fri3d_util.c"
)

set(DEPS
        "fri3d_bsp"
)

set(PRIV_DEPS
        ""
)

if (CONFIG_FRI3D_BUZZER)
    list(APPEND SRCS
        "src/rtttl/rtttl.c"
    )
endif ()

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
