obj-m := hello.o

ARCH = arm
CROSS_COMPILE = arm-linux-gnueabihf-
KDIR = /home/sirisha/linux-bbb-5.10
PWD :=$(shell pwd) 
all:
	make -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
clean:
	make -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) clean

