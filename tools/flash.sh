#/bin/bash

esptool.py --chip esp32s3 -p /dev/ttyACM0 -b1500000 --before=default_reset --after=hard_reset write_flash --erase-all --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 images/bootloader.bin 0x8000 images/partition-table.bin 0x9000 images/ota_data_initial.bin 0x10000 images/fri3d_firmware_fox.bin 0x210000 images/fri3d_firmware_fox.bin 0x410000 images/hwtest.bin 0x710000 images/launcher.bin 0x810000 images/retro-core.bin 0x8b0000 images/prboom-go.bin 0x990000 images/vfs.bin

