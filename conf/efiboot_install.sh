#!/usr/bin/bash
sudo rm -rfv /boot/EFI/ChengChengOS
sudo cp -rfv /home/0d00/src/ccoskrnl/esp/EFI/ChengChengOS /boot/EFI/
sudo cp -fv /home/0d00/src/ccoskrnl/esp/EFI/BOOT/BOOTX64.EFI /boot/EFI/ChengChengOS/ccbootx64.efi