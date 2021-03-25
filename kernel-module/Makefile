ccflags-y = -std=gnu99 -fno-pie -Wframe-larger-than=2048
obj-m += flagchecker.o
flagchecker-objs := main.o

all:
	make -C  /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
