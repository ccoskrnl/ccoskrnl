#!/usr/bin/bash
sudo rm -rfv /boot/EFI/ChengChengOS
sudo cp -rfv /home/cc/projects/ccoskrnl/esp/EFI/ChengChengOS /boot/EFI/
sudo cp -fv /home/cc/projects/ccoskrnl/esp/EFI/BOOT/BOOTX64.EFI /boot/EFI/ChengChengOS/ccbootx64.efi