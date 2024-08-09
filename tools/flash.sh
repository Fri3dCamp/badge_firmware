#/bin/bash

badge="${1:-fox}"

case $badge in

  fox)
    device=/dev/ttyACM0
    ;;

  octopus)
    device=/dev/ttyUSB0
    ;;

  *)
    echo "Unknown type"
    exit
    ;;
esac

esptool.py --chip esp32s3 -p ${device} -b1500000 --before=default_reset --after=hard_reset write_flash --erase-all --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 images/bootloader_${badge}.bin 0x8000 images/partition_table_${badge}.bin 0x9000 images/ota_data_${badge}.bin 0x10000 images/fri3d_firmware_${badge}.bin 0x210000 images/fri3d_firmware_${badge}.bin 0x410000 images/micropython_${badge}.bin 0x710000 images/retro_go_launcher_${badge}.bin 0x810000 images/retro_go_core_${badge}.bin 0x8b0000 images/retro_go_prboom_${badge}.bin 0x990000 images/vfs_${badge}.bin
