#!/bin/bash
# legt /-Verzeichnisse bzw. -Dateien des Zielsystems an
MAINDIR=~/embedded/raspi

cd $MAINDIR/userland
dd if=/dev/zero of=rootfs.img bs=1k count=16384
sudo mkfs.ext2 -i 1024 -F rootfs.img
cd $MAINDIR/
sudo mount -o loop userland/rootfs.img userland/loop
sudo rsync -a userland/busybox-1.26.2/_install/ userland/loop
cd $MAINDIR/userland
sudo ln -s sbin/init loop/init
mkdir loop/dev
mkdir loop/etc
mkdir loop/proc
mkdir loop/sys
sudo chown -R root.root loop/*
sudo mknod loop/dev/null c 1 3
sudo mknod loop/dev/tty1 c 4 1
sudo mknod loop/dev/console c 5 1
sudo mknod loop/dev/ttyAMA0 c 204 64

# aufgabe 8
cd $MAINDIR/userland
sudo mkdir -p loop/var/www
sudo cp ./target/rcS loop/etc
sudo chmod +x loop/etc/rcS
#sudo cp ../target/httpd.conf loop/etc
#sudo cp ../target/index.html loop/var/www
#sudo cp ../target/ps.cgi loop/var/www/
#sudo chmod +x loop/var/www/ps.cgi
sudo install -m 644 ./target/inittab loop/etc
sudo install -m 644 ./target/httpd.conf loop/etc
sudo install -m 644 ./target/index.html loop/var/www
sudo install -m 755 ./target/ps.cgi loop/var/www
sudo chown -R root.root loop/*

#sudo mkdir loop/sysfs

(cd loop; sudo find . | cpio -o -H newc | gzip -9) > initramfs.cpio.gz
mkimage -A arm -O linux -T RAMDISK -C none -a 0 -n "Root Filesystem" -d initramfs.cpio.gz initramfs.uboot
sudo cp initramfs.uboot /var/lib/tftpboot
# linux image: zImage
sudo umount loop


