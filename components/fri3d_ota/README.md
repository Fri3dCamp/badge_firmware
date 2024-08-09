# Fri3d ota component

The ota update app on the Fri3d badges, allows you to update the partitions.

## Update metadata

The firmware fetches update metadata from a URL that is configured in `FRI3D_VERSIONS_URL`. The URL points to a .json
file that contains all the information for the format compatible firmware images. If a new format is introduced, a new
.json file is configured. This file can contain the last firmware that used the previous format to allow for rollback.

### v00

* https://fri3d.be/firmware/firmware-octopus.json
* https://fri3d.be/firmware/firmware-fox.json

The first version released in the wild. As it and the related code was created under time pressure, the format is very
spartan and (unfortunately) does not allow a lot of flexibility.

```json
[
  {
    "version": "0.1.5-esp-idf.1+build.0",
    "url": "https://github.com/MathyV/fri3d_firmware/releases/download/v0.1.5/fri3d_firmware_fox.bin",
    "size": 1395200
  }
]
```

Version `0.1.5` requires each entry to have at least these fields. We can reasonably expect everyone that visited the
2024 camp to have updated to at least `v0.1.6` as this version was not really very functional, but still, it is here for
historical reasons. There might still be Fox badges that did not have the upgrade. Octopus badges that were flashed at
the 2024 camp should also not have this version.

### v01

* https://fri3d.be/firmware/v01/firmware-octopus.json
* https://fri3d.be/firmware/v01/firmware-fox.json

A much more functional format created once the extra OTA functionality was developed.

```json
[
  {
    "version": "0.1.6",
    "beta": true,
    "images": [
      {
        "type": "main",
        "version": "0.1.6-esp-idf.1+build.0",
        "url": "https://github.com/MathyV/fri3d_firmware/releases/download/v0.1.6/fri3d_firmware_fox.bin",
        "size": ...
      },
      {
        "type": "micropython",
        "version": "...",
        "url": "...",
        "size": ...
      }
    ]
  }
]
```

It contains the following fields:
* `version`: combined firmware image version, this has to match the main part of the version of the main image
* `beta`: whether this is a beta firmware, used for filtering
* `images`: images that need to be flashed to other partitions on the badge. The software will not flash same versions
    as already installed unless the main firmware is also being flashed to its same version (=reinstall)
  * `type`: the type of image
    * `bootloader`: Currently not flashed by firmware, used by webflasher
    * `partition`: Currently not flashed by firmware, used by webflasher
    * `ota-data`: Currently not flashed by firmware, used by webflasher
    * `main`: The main firmware image
    * `micropython`: MicroPython
    * `retro-launcher`: Launcher for Retro-Go
    * `retro-core`: Core of Retro-Go
    * `retro-prboom`: Doom port for Retro-Go
    * `vfs`: Fat filesystem for user files
  * `version`: version of the image
  * `url`: where to get the image
  * `size`: size of the image
