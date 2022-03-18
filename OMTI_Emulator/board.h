#ifndef _BOARD_H_
#define _BOARD_H_

#define LED1_INIT	DDRD |= (1 << PD6); LED1_OFF	
#define LED1_ON		PORTD |= (1 << PD6);
#define LED1_OFF	PORTD &= ~(1 << PD6);
#define LED1		LED1_ON LED1_OFF

#define SW1_INIT	DDRD &= ~(1 << PD2); PORTD |= (1 << PD2);
#define SW1_INPUT	(PIND & (1 << PD2))

#define SW2_INIT	DDRD &= ~(1 << PD3); PORTD |= (1 << PD3);
#define SW2_INPUT	(PIND & (1 << PD3))

#endif /* _BOARD_H_ */
