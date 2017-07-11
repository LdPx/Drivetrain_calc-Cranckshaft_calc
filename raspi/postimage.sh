#!/bin/bash
sudo cp ~/embedded/raspi/buildroot-2016.02/output/images/rootfs.cpio.uboot /var/lib/tftpboot/initramfs.uboot
sudo cp ~/embedded/raspi/buildroot-2016.02/output/images/zImage /var/lib/tftpboot/kernel7.img
sudo install -m 755 ~/embedded/raspi/follow/follow /var/lib/tftpboot

