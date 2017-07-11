ifneq ($(KERNELRELEASE),)
obj-m := sensor_treiber.o

else
KDIR := /local_home/embedded/raspi/buildroot-2016.02/output/build/linux-d33d0293e245badc4ca6ede3984d8bb8ea63cb1a/
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf Module.symvers sensor_treiber.ko sensor_treiber.mod.o modules.order sensor_treiber.mod.c sensor_treiber.o


endif
