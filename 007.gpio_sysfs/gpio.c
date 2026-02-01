#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "gpio.h"

/* ---------- internal helpers ---------- */
static int write_file(const char *path, const char *val)
{
    int fd = open(path, O_WRONLY);
    if(fd < 0) return -1;
    write(fd, val, strlen(val));
    close(fd);
    return 0;
}

static int read_file(const char *path)
{
    char c;
    int fd = open(path, O_RDONLY);
    if(fd < 0) return -1;
    read(fd, &c, 1);
    close(fd);
    return c - '0';
}

/* ---------- public API ---------- */

int gpio_export(int gpio)
{
    char path[] = "/sys/class/gpio/export";
    char num[8];
    sprintf(num, "%d", gpio);
    return write_file(path, num);
}

int gpio_unexport(int gpio)
{
    char path[] = "/sys/class/gpio/unexport";
    char num[8];
    sprintf(num, "%d", gpio);
    return write_file(path, num);
}

int gpio_set_dir(int gpio, int is_output)
{
    char path[64];
    sprintf(path, "/sys/class/gpio/gpio%d/direction", gpio);
    return write_file(path, is_output ? "out" : "in");
}

int gpio_write(int gpio, int value)
{
    char path[64];
    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio);
    return write_file(path, value ? "1" : "0");
}

int gpio_read(int gpio)
{
    char path[64];
    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio);
    return read_file(path);
}
