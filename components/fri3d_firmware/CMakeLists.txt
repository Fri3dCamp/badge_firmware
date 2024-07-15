include(${CMAKE_BINARY_DIR}/../../../cmake/fri3d_component.cmake)

set(SRCS
        "src/main.cpp"
)

set(PRIV_DEPS
        "fri3d_bsp"
        "fri3d_application"
        "fri3d_launcher"
)

idf_component_register(
        SRCS
        "${SRCS}"
        PRIV_REQUIRES
        "${PRIV_DEPS}"
)

fri3d_set_loglevel()
