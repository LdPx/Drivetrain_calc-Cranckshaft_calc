#!/bin/bash
sudo hexdump -n 10000 -cv /dev/mydevzero  | grep -o "\\\0" | wc  -l

