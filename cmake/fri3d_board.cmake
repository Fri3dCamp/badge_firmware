# Add our shared components dir
set(EXTRA_COMPONENT_DIRS "../../components")

# We make sure the COMPONENTS variable is clear, this has some advantages:
# * The build is smaller
# * It forces developers to properly define dependencies
# * We can have multiple bsp's in the components directory that don't get built at the same time
if (NOT DEFINED COMPONENTS)
    # Make sure we actually build the fri3d_firmware component
    set(COMPONENTS "fri3d_firmware")
endif ()

# Generated sdkconfig files are build artifacts, prove me wrong
set(SDKCONFIG "${CMAKE_CURRENT_BINARY_DIR}/sdkconfig")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# This is a temporary workaround until 5.2.3 (or higher) gets released:
# https://github.com/espressif/esp-idf/issues/14157
# We inject components that have a idf_component.yml file into the top-level COMPONENTS in order for the component
# manager to work
list(APPEND COMPONENTS "fri3d_bsp fri3d_application")
