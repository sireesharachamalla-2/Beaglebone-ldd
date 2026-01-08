obj-m += sample.o

ARCH = arm
CROSS_COMPILE = arm-linux-gnueabihf-
KDIR = /home/sirisha/linux-bbb-5.10
PWD :=$(shell pwd) 

all:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

