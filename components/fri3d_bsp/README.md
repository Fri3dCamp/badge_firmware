# BSP package for Fri3d Camp badges

This component provides the *Board Support Package* for badges that are handed out at Fri3d Camp.

## esp-bsp

The BSP borrows some ideas from [esp-bsp](https://github.com/espressif/esp-bsp), but it is not ABI-compatible, for two
reasons:

* The badges share pretty similar designs, and thus share code
* We don't agree with all the design decisions made in `esp-bsp`
