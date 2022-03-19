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
#include "config.h"
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

	Config_Init();

	sei();

	extern void sd_card_init(void);
	sd_card_init();

	printf("\r\nStart OMTI Emulator\r\n");

    while (1) {

		// Nacteni stavu tlacitek

		BUTTONS buttons = GetButtons();

		uint8_t sd_status = sd_card_proc();

		if(sd_status & SD_STATE_CHANGE) {
			
			// Spusteni procesu konfigurace pri zmene stavu SD karty
			Config_Proc(sd_status);
		}

		scsi_proc();
	
		lcd_proc(buttons);
	
    }
}

