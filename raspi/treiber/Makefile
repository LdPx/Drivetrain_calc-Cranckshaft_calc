ifneq ($(KERNELRELEASE),)
obj-m := rpi2_motor.o

else
#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /local_home/embedded/raspi/buildroot-2016.02/output/build/linux-d33d0293e245badc4ca6ede3984d8bb8ea63cb1a/
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
endif
