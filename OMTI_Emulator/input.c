/*
 * input.c
 *
 * Created: 17.03.2022 19:31:57
 *  Author: marek.hummel
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "board.h"
#include "Input.h"

#define BUTTON_NUM	2

volatile static uint8_t input_value = 0;		// Aktualni hodnota vstupu, co bit to vstup
volatile static uint8_t input_change = 0;		// Priznak zmeny vstupu, co bit to vstup

BUTTONS GetButtons(void)
{
	BUTTONS buttons;

	cli();
	buttons.value = input_value;
	buttons.change = input_change;
	input_change = 0;
	sei();
	
	return buttons;
}


//*****************************************************************************
// Funkce volana casovacem urcena pro cteni stavu a zmeny stisknuti tlacitek
// s protizakmitem
//*****************************************************************************
 void Input_Timer10mS_Tick(void)
 {
	 volatile static uint8_t input_glitch[BUTTON_NUM]			= {0, 0};		// Pomocny buffer pro potlaceni zakmitu
	 uint8_t i;
	 
	 // Fyzicke precteni vstupu s vlozenim a rotaci glitch bufferu
	 input_glitch[0] = (input_glitch[0] << 1) | (SW1_INPUT ? 0 : 1);		// Leve tlacitko
	 input_glitch[1] = (input_glitch[1] << 1) | (SW2_INPUT ? 0 : 1);		// Prave tlacitko
	 
	 // Detektor hran
	 for(i = 0; i < BUTTON_NUM; i++) {

		 // Nabezna hrana
		 if((input_glitch[i] & 0x7) == 0x3) {		/* 011 */
			 input_value |= (1 << i);				// stav
			 input_change |= (1 << i);				// priznak zmeny
		 }

		 // Spadova hrana
		 else if((input_glitch[i] & 0x7) == 0x4) {	/* 100 */
			 input_value &= ~(1 << i);				// stav
			 input_change |= (1 << i);				// priznak zmeny
		 }
	 }
 }
 
 
  void Input_Init(void)
 {
	SW1_INIT;
	SW2_INIT;
 }
 
 
 
void Input_Test(void)
{
	BUTTONS buttons = GetButtons();
	 
	if(buttons.change & BUTTON_SW1) {
		if(buttons.value & BUTTON_SW1)	{
			printf("Button 1 on\r\n");
		} else {
			printf("Button 1 off\r\n");			
		}
	}

	if(buttons.change & BUTTON_SW2) {
		if(buttons.value & BUTTON_SW2)	{
			printf("Button 2 on\r\n");
		} else {
			printf("Button 2 off\r\n");
		}
	}
 }
 
 
