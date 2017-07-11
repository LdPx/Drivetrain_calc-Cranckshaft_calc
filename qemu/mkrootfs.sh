#!/bin/bash
# legt /-Verzeichnisse bzw. -Dateien des Zielsystems an
MAINDIR=~/embedded

cd $MAINDIR/qemu/userland
dd if=/dev/zero of=rootfs.img bs=1k count=16384
sudo mkfs.ext2 -i 1024 -F rootfs.img
cd $MAINDIR/qemu
sudo mount -o loop userland/rootfs.img userland/loop
sudo rsync -a userland/busybox-1.26.2/_install/ userland/loop
cd $MAINDIR/qemu/userland
mkdir loop/dev
mkdir loop/etc
mkdir loop/proc
mkdir loop/sys
sudo mknod loop/dev/null c 1 3
sudo mknod loop/dev/tty1 c 4 1
sudo mknod loop/dev/console c 5 1

# gegen warnings
sudo mknod loop/dev/tty2 c 4 2
sudo mknod loop/dev/tty3 c 4 3
sudo mknod loop/dev/tty4 c 4 4
sudo mkdir loop/etc/init.d

sudo chown -R root.root loop/*
# ???
#sudo umount loop

# aufgabe 8
cd $MAINDIR/qemu/userland
sudo mkdir loop/etc
sudo mkdir -p loop/var/www
#sudo cp ../target/rcS loop/etc/init.d
#sudo chmod +x loop/etc/init.d/rcS

sudo cp ../target/inittab loop/etc
sudo cp ../target/rcS loop/etc
sudo chmod +x loop/etc/rcS

sudo cp ../target/httpd.conf loop/etc
sudo cp ../target/index.html loop/var/www
sudo cp ../target/ps.cgi loop/var/www/
sudo chmod +x loop/var/www/ps.cgi

sudo mkdir loop/sysfs
sudo umount loop

