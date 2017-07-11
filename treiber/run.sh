#!/bin/bash

make
sudo insmod 1/mod1.ko
tail -f /var/log/kern.log
#sudo rmmod 1/mod1


