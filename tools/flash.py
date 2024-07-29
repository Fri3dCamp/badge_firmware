# heavily inspired on https://github.com/Fri3dCamp/Fri3dBadge/blob/master/bin/defaultFirmware/flash.py
import sys
import glob
import subprocess
import time

from serial.tools import list_ports

# idVendor=303a, idProduct=1001
VID = 0x303A
PID = 0x1001


def get_serial():
    device_list = list_ports.comports()
    devices = []
    for device in device_list:
        if (device.vid != None or device.pid != None):
            if (device.vid == VID and
                    device.pid == PID):
                #print('found port ' + device.device)
                devices.append(device)

    return devices


seen_devices = []
while True:
    attached = get_serial()
    for port in attached:
        if port.serial_number not in seen_devices:
            seen_devices.append(port.serial_number)

            subprocess.Popen([
                "esptool.py", "--chip", "esp32s3", "-b1500000", "-p", port.device, "--before", "default_reset", "--after",
                "hard_reset",
                "write_flash", "--erase-all", "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "16MB",
                "0x0", "images/bootloader.bin",
                "0x8000", "images/partition-table.bin",
                "0x9000", "images/ota_data_initial.bin",
                "0x10000", "images/fri3d_firmware_fox.bin",
                "0x210000", "images/fri3d_firmware_fox.bin",
                "0x410000", "images/hwtest.bin",
                "0x710000", "images/launcher.bin",
                "0x810000", "images/retro-core.bin",
                "0x8b0000", "images/prboom-go.bin",
                "0x990000", "images/vfs.bin"
            ]
            )
    time.sleep(1)
