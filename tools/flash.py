# heavily inspired on https://github.com/Fri3dCamp/Fri3dBadge/blob/master/bin/defaultFirmware/flash.py
import sys
import glob
import subprocess
import time
import argparse

from serial.tools import list_ports

# idVendor=303a, idProduct=1001


def get_serial(badge):
    if (badge == "fox"):
        VID = 0x303A
        PID = 0x1001
    elif (badge == "octopus"):
        VID = 0x10C4
        PID = 0xEA60

    device_list = list_ports.comports()
    devices = []
    for device in device_list:
        if (device.vid != None or device.pid != None):
            if (device.vid == VID and
                    device.pid == PID):
                #print('found port ' + device.device)
                devices.append(device)

    return devices


def flash(badge):
    seen_devices = []
    while True:
        attached = get_serial(badge)
        for port in attached:
            if port.serial_number not in seen_devices:
                seen_devices.append(port.serial_number)

                subprocess.Popen([
                    "esptool.py", "--chip", "esp32s3", "-b1500000", "-p", port.device, "--before", "default_reset", "--after",
                    "hard_reset",
                    "write_flash", "--erase-all", "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "16MB",
                    "0x0", f"images/bootloader_{badge}.bin",
                    "0x8000", f"images/partition_table_{badge}.bin",
                    "0x9000", f"images/ota_data_{badge}.bin",
                    "0x10000", f"images/fri3d_firmware_{badge}.bin",
                    "0x210000", f"images/fri3d_firmware_{badge}.bin",
                    "0x410000", f"images/micropython_{badge}.bin",
                    "0x710000", f"images/retro_go_launcher_{badge}.bin",
                    "0x810000", f"images/retro_go_core_{badge}.bin",
                    "0x8b0000", f"images/retro_go_prboom_{badge}.bin",
                    "0x990000", f"images/vfs_{badge}.bin"
                ]
                )
        time.sleep(1)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('badge', nargs='?', default='fox', choices=['fox', 'octopus'])

    args = parser.parse_args()

    flash(args.badge)
