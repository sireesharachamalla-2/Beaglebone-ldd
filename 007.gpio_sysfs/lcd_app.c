#include "lcd.h"
#include "gpio.h"
#include <unistd.h>

int main()
{
    struct lcd lcd = {
        .rs = 54,    // gpio1_22
        .rw = 74,    // gpio2_10  <-- IMPORTANT FOR BUSY FLAG
        .en = 55,    // gpio1_23
        .d4 = 70,    // gpio2_6
        .d5 = 71,    // gpio2_7
        .d6 = 72,    // gpio2_8
        .d7 = 73     // gpio2_9
    };

    int *pins = &lcd.rs;
    for (int i = 0; i < 7; i++) {
        gpio_export(pins[i]);
        gpio_set_dir(pins[i], 1);
    }

    lcd_init(&lcd);
    lcd_print(&lcd, "  HD44780 16x2 ");
    lcd_print(&lcd, "   BeagleBone  ");

    return 0;
}
