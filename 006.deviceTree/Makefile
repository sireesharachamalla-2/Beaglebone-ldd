obj-m += main.o

ARCH = arm
CROSS_COMPILE = arm-linux-gnueabihf-
KDIR = /home/sirisha/linux-bbb-5.10
PWD :=$(shell pwd)
HOST_KERN_DIR=/lib/modules/$(shell uname -r)/build/
all:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) m>
clean:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) c>
host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules
copy-dtb:
	scp /home/sirisha/linux-bbb-5.10/arch/arm/boot/dts/am335x-boneblack.dtb debi>
copy-drv:
	scp *.ko *.elf debian@192.168.7.2:/home/debian/bbb

