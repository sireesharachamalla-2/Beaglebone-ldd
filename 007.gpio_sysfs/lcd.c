#include <unistd.h>
#include "lcd.h"
#include "gpio.h"

/* ---- internal helpers ---- */

static void pulse(struct lcd *lcd)
{
    gpio_write(lcd->en, 1);
    usleep(1);
    gpio_write(lcd->en, 0);
    usleep(50);
}

/* read upper nibble; D7 is BF */
static unsigned char read_nibble(struct lcd *lcd)
{
    unsigned char val = 0;
    val |= gpio_read(lcd->d7) << 3;
    val |= gpio_read(lcd->d6) << 2;
    val |= gpio_read(lcd->d5) << 1;
    val |= gpio_read(lcd->d4) << 0;
    return val;
}

/* ---- step-8 busy-flag polling ---- */
static void lcd_wait(struct lcd *lcd)
{
    unsigned char busy;

    gpio_set_dir(lcd->d7, 0);   // D7 input for BF
    gpio_write(lcd->rw, 1);     // read mode
    gpio_write(lcd->rs, 0);     // command register

    do {
        pulse(lcd);
        busy = read_nibble(lcd) & 0x08; // check BF bit
        pulse(lcd);                     // read lower nibble (ignored)
    } while (busy);

    gpio_write(lcd->rw, 0);     // back to write mode
    gpio_set_dir(lcd->d7, 1);   // D7 -> output again
}

/* ---- send byte ---- */
static void send(struct lcd *lcd, unsigned char data, int rs)
{
    lcd_wait(lcd);

    gpio_write(lcd->rs, rs);

    gpio_write(lcd->d4, (data >> 4) & 1);
    gpio_write(lcd->d5, (data >> 5) & 1);
    gpio_write(lcd->d6, (data >> 6) & 1);
    gpio_write(lcd->d7, (data >> 7) & 1);
    pulse(lcd);

    gpio_write(lcd->d4, (data >> 0) & 1);
    gpio_write(lcd->d5, (data >> 1) & 1);
    gpio_write(lcd->d6, (data >> 2) & 1);
    gpio_write(lcd->d7, (data >> 3) & 1);
    pulse(lcd);
}

/* ---- public API ---- */

void lcd_init(struct lcd *lcd)
{
    gpio_write(lcd->rw, 0); // default write
    usleep(15000);

    send(lcd, 0x33, 0);
    send(lcd, 0x32, 0);
    send(lcd, 0x28, 0);
    send(lcd, 0x0C, 0); // disp on, cursor off
    send(lcd, 0x06, 0); // entry mode
    send(lcd, 0x01, 0); // clear
}

void lcd_print(struct lcd *lcd, const char *msg)
{
    while (*msg)
        send(lcd, *msg++, 1);
}
