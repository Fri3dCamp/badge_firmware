cmake_minimum_required(VERSION 3.16)

include(../../cmake/fri3d_board.cmake)

set(SDKCONFIG_DEFAULTS
        ../shared/sdkconfig.base
        ../shared/sdkconfig.240mhz
        ../shared/sdkconfig.flash.16mb
        ../shared/sdkconfig.spiram_oct

        ../shared/sdkconfig.joystick
        ../shared/sdkconfig.lvgl

        sdkconfig.defaults
)

project(fri3d_firmware_fox)
