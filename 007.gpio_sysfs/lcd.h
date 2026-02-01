#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>

struct lcd {
    int rs, rw, en;
    int d4, d5, d6, d7;
};

void lcd_init(struct lcd *lcd);
void lcd_print(struct lcd *lcd, const char *msg);

#endif
