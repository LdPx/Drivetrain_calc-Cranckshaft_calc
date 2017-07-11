#!/bin/bash
mkimage -A arm -O linux -T SCRIPT -C none -a 0 -n "Root Filesystem" -d boot.txt boot.scr
# boot.scr in Flash 
