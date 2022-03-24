/*
 * timer.c
 *
 * Created: 05.03.2022 20:20:45
 *  Author: marek.hummel
 */ 

#include <avr/interrupt.h>
#include "board.h"
#include "timer.h"
#include "FatFs/mmc_avr.h"
#include "lcd.h"
#include "input.h"

volatile uint16_t timer10mS;

// Timer 10mS
ISR(TIMER0_COMPA_vect)
{
//LED1_ON	
	Input_Timer10mS_Tick();
	
	lcd_timer_10mS();	

	mmc_disk_timerproc();	/* Drive timer procedure of low level disk I/O module */
//LED1_OFF
}


void timer_init(void)
{
	/* Start 100Hz(10mS) system timer with TC0 */
	OCR0A = F_CPU / 1024 / 100;
	// CTC mode, OCRA top
	TCCR0A = (1 << WGM01);
	TCCR0B = PRESCALE0_1024;
	TIMSK0 = (1 << OCIE0A); // User	
}
