#ifndef _GPIO_H_
#define _GPIO_H_

/* Userspace GPIO sysfs access API */

int gpio_export(int gpio);
int gpio_unexport(int gpio);
int gpio_set_dir(int gpio, int is_output);
int gpio_write(int gpio, int value);
int gpio_read(int gpio);

#endif
