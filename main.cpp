#include <pico/stdlib.h>
#include "hd44780.h"
#include <string.h>

int main()
{
    PIO pio = pio0;
    int sm = 0;
    int pin = 16;
    float freq = 2000000;

    hd44780_init(pio, sm, pin, freq);   

    hd44780_functionset(false, true, false);
    hd44780_cleardisplay();
    hd44780_displaycontrol(true, false, false);

    char buf[64];

    int i = 0;
    while(true)
    {
        sprintf(buf, "--%08d%08d--", i, i);
        i++;

        hd44780_setddramaddr(0x00);
        hd44780_writebytes((const uint8_t *)buf, strlen(buf));

        hd44780_setddramaddr(0x40);
        hd44780_writebytes((const uint8_t *)buf, strlen(buf));

        hd44780_setddramaddr(0x14);
        hd44780_writebytes((const uint8_t *)buf, strlen(buf));

        hd44780_setddramaddr(0x54);
        hd44780_writebytes((const uint8_t *)buf, strlen(buf));
    }
}