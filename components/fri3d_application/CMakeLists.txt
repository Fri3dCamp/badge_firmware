include(${CMAKE_BINARY_DIR}/../../../cmake/fri3d_component.cmake)

set(SRCS
        "src/app.cpp"
        "src/app_manager.cpp"
        "src/application.cpp"
        "src/lvgl.cpp"
        "src/indev.cpp"
        "src/indev_buttons.cpp"
)

if (CONFIG_FRI3D_JOYSTICK)
    list(APPEND SRCS
        "src/indev_joystick.cpp"
    )
endif ()

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
        PRIV_INCLUDE_DIRS
        "src/include"
        REQUIRES
        "${DEPS}"
        PRIV_REQUIRES
        "${PRIV_DEPS}"
)

fri3d_set_loglevel()
