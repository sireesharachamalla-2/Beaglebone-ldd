obj-m += gpio-driver.o

ARCH = arm
CROSS_COMPILE = arm-linux-gnueabihf-
KDIR = /home/sirisha/linux-bbb-5.10
PWD :=$(shell pwd)
HOST_KERN_DIR=/lib/modules/$(shell uname -r)/build/
all:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	make -C $(KDIR) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules
copy-dtb:
	scp /home/sirisha/linux-bbb-5.10/arch/arm/boot/dts/am335x-boneblack.dtb debian@192.168.7.2:/home/debian/bbb
copy-drv:
	scp *.ko *.elf debian@192.168.7.2:/home/debian/bbb
app: lcd_app.o lcd.o gpio.o lcd_app.elf

lcd_app.o: lcd_app.c
	$(CROSS_COMPILE)gcc -c -o $@ $^

lcd.o: lcd.c
	$(CROSS_COMPILE)gcc -c -o $@ $^
	
gpio.o: gpio.c
	$(CROSS_COMPILE)gcc -c -o $@ $^

lcd_app.elf: lcd_app.o lcd.o gpio.o
	$(CROSS_COMPILE)gcc -o $@ $^
