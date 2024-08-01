cmake_minimum_required(VERSION 3.16)

include(../../cmake/fri3d_board.cmake)

set(SDKCONFIG_DEFAULTS
        ../shared/sdkconfig.base
        ../shared/sdkconfig.240mhz
        ../shared/sdkconfig.flash.16mb
        ../shared/sdkconfig.spiram_quad
        ../shared/sdkconfig.wifi

        ../shared/sdkconfig.lvgl

        sdkconfig.defaults
)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.local")
    message("Found sdkconfig.local, adding")
    list(APPEND SDKCONFIG_DEFAULTS "sdkconfig.local")
endif ()

project(fri3d_firmware_octopus)
