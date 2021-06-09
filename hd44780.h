#ifndef HD44780_H
#define HD44780_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <pico.h>
#include <hardware/pio.h>

/*!
 * @brief initialization
 */
void hd44780_init(PIO pio, int sm);

/*!
 * @brief termination
 */
void hd44780_term();

/*!
 * @brief clear display
 */
void hd44780_cleardisplay();

/*!
 * @brief return home
 */
void hd44780_returnhome();

/*!
 * @brief return home
 * @param id true:  cursor moves to right and DDRAM address is increased by 1, 
 *           false: cursor moves to left and DDRAM address is decreased by 1
 * @param s TODO: 
 */
void hd44780_entrymodeset(bool id, bool s);

/*!
 * @brief display control
 * @param d display on
 * @param c cursor on
 * @param b cursor blink
 */
void hd44780_displaycontrol(bool d, bool c, bool b);

/*!
 * @brief cursor or display shift
 * @param sc false = shift cursor, true = shift display
 * @param rl false = left, true = right
 */
void hd44780_cursordisplayshift(bool sc, bool rl);

/*!
 * @brief function set
 * @param dl false = 4-bit mode, true = 8-bit mode
 * @param n false = 1-line display mode, true = 2-line display mode
 * @param f false = 5x8 font, true = 5x11 font
 */
void hd44780_functionset(bool dl, bool n, bool f);

/*!
 * @brief set CGRAM address
 * @param addr address (0-63)
 */
void hd44780_setcgramaddr(uint8_t addr);

/*!
 * @brief set CGRAM address
 * @param addr address (0-127)
 */
void hd44780_setddramaddr(uint8_t addr);

/*!
 * @brief write single byte to RAM 
 * @param value value
 */
void hd44780_writebyte(uint8_t value);

/*!
 * @brief write bytes to RAM 
 * @param value addr
 * @param len length
 */
void hd44780_writebytes(const uint8_t *addr, int len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // HD44780_H