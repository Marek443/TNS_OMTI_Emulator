/*
 * lcd.h
 *
 * Created: 10.03.2022 21:46:13
 *  Author: marek.hummel
 */ 


#ifndef LCD_H_
#define LCD_H_

#include "input.h"

void lcd_init(void);
void lcd_proc(BUTTONS buttons);
void lcd_timer_10mS(void);

#endif /* LCD_H_ */
