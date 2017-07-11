#!/bin/sh

# MARK A: Netzwerk starten
install -m 755 ../userland/target/S41udhcpc $1/etc/init.d/

# MARK B: Zeit setzen
install -m 755 ../userland/target/S52ntp $1/etc/init.d/

# MARK C: SD-Karte einhaengen
echo "SD-Karte einhaengen..."
mkdir -p $1/boot
mkdir -p $1/data
if ! grep "/dev/mmcblk0p" -q $1/etc/fstab; then
    echo "/dev/mmcblk0p1 /boot  vfat  defaults  0  0" >> $1/etc/fstab
    echo "/dev/mmcblk0p2 /data  ext4  defaults  0  0" >> $1/etc/fstab
fi

# MARK D: Webserver
sudo mkdir -p $1/var/www
sudo cp ../userland/target/rcS $1/etc
sudo chmod +x $1/etc/rcS
sudo install -m 644 .../userland/target/inittab $1/etc
sudo install -m 644 ../userland/target/httpd.conf $1/etc
sudo install -m 644 ../userland/target/index.html $1/var/www
sudo install -m 755 ../userland/target/ps.cgi $1/var/www

# MARK E: Firewall

# MARK F: Usermanagement
sudo install -m 755 ../treiber/set_gpios.sh $1/root
cp ../treiber/rpi2_motor.ko $1/root
cp ../treiber/sensor_treiber/sensor_treiber.ko $1/root
install -m 755 ../treiber/drive/drive $1/root
install -m 755 ../treiber/sensor_treiber/test/sensor_test $1/root
install -m 755 ../follow/follow $1/root
