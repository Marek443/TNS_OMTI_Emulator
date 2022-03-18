/*
 * OMTI_Emulator.c
 *
 * Created: 27.02.2022 20:21:29
 * Author : marek.hummel
 */ 

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "board.h"
#include "timer.h"
#include "serial0.h"
#include "scsi.h"
#include "sd_card.h"
#include "lcd.h"
#include "input.h"



int main(void)
{
	LED1_INIT;

	LED1;
	LED1;
	LED1;

	Input_Init();

	timer_init();
	
	Uart0_Init(115200);

	lcd_init();


	sei();

	extern void sd_card_init(void);
	sd_card_init();

	printf("\r\nStart OMTI Emulator\r\n");

    while (1) {

		// Nacteni stavu tlacitek
		BUTTONS buttons = GetButtons();

		scsi_proc();
	
		sd_card_proc();
		
		lcd_proc(buttons);
		
		
		extern void cfg_proc(BUTTONS buttons);
		cfg_proc(buttons);
	
    }
}

