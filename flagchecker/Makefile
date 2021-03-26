KERNEL_PATH ?= /lib/modules/$(shell uname -r)/build
ccflags-y = -std=gnu99 -fno-pie -Wframe-larger-than=2048
obj-m += flagchecker.o

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean
