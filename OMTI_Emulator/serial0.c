#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial0.h"
#include "board.h"

/* RXD0  - PD0 */
/* TXD0  - PD1 */

// *****************************************************************************
// Konfigurace serioveho bufferu
// *****************************************************************************
#define TX_UART0_BUF_SIZE	2048
#define TX_UART0_MASK			(TX_UART0_BUF_SIZE - 1)
#define RX_UART0_BUF_SIZE	128
#define RX_UART0_MASK			(RX_UART0_BUF_SIZE - 1)


volatile static uint8_t		uart0_TxBuf[TX_UART0_BUF_SIZE];
volatile static uint16_t	uart0_TxHead = 0;
volatile static uint16_t	uart0_TxTail = 0;
volatile static uint8_t		uart0_RxBuf[RX_UART0_BUF_SIZE];
volatile static uint8_t		uart0_RxHead = 0;
volatile static uint8_t		uart0_RxTail = 0;
volatile static uint8_t		uart0_RxCount = 0;


//******************************************************************************
// Serial Transmit IRQ
//******************************************************************************
ISR(USART0_UDRE_vect)
{
	if(uart0_TxTail != uart0_TxHead) {
		UDR0 = uart0_TxBuf[uart0_TxTail++];
		uart0_TxTail &= TX_UART0_MASK;
	} else {
		UCSR0B &= ~(1 << UDRIE0);
	}
}

// *****************************************************************************
// Serial 0 Receive IRQ
// *****************************************************************************
ISR(USART0_RX_vect)
{
	unsigned char tmp = UDR0;
	if (uart0_RxCount != RX_UART0_BUF_SIZE) {
		uart0_RxBuf[uart0_RxHead] = tmp;
		uart0_RxCount++;
		uart0_RxHead++;
		uart0_RxHead &= RX_UART0_MASK;
	}
}


int Uart0_NoBlock_GetChar(void)
{
    char c = 0;
	if (uart0_RxCount != 0) {

		cli();
		uart0_RxCount--;
		sei();

		c = uart0_RxBuf[uart0_RxTail];
		uart0_RxTail++;
		uart0_RxTail &= RX_UART0_MASK;
		return c;
	} else {
		return -1;
	}
}


uint8_t uart0_RxTest(void)
{
	return (uart0_RxCount == 0) ? 0 : 1;
}


void uart0_Rx_Clear(void)
{
	cli();
	uart0_RxHead = 0;
	uart0_RxTail = 0;
	uart0_RxCount = 0;
	sei();
}


int Uart0_PutChar(char c, FILE *fd)
{
	// Zjisteni zda je v bufferu misto
	while(uart0_TxTail == ((uart0_TxHead + 1) & TX_UART0_MASK)) {
		// ne neni, zahodit znak
LED1
		return -1;
	}

	// Vlozeni znaku do bufferu
	uart0_TxBuf[uart0_TxHead++] = c;
	uart0_TxHead &= TX_UART0_MASK;

	// Povoleni IRQ
	UCSR0B |= (1 << UDRIE0);

	return c;
}


// *****************************************************************************
// UART init routine
// *****************************************************************************
void Uart0_Init(unsigned long baud)
{
	uart0_TxHead = 0;
	uart0_TxTail = 0;
	uart0_RxHead = 0;
	uart0_RxTail = 0;
	uart0_RxCount = 0;

	memset((char*)uart0_TxBuf, 0, sizeof(uart0_TxBuf));
	memset((char*)uart0_RxBuf, 0, sizeof(uart0_RxBuf));

	unsigned short sv = (unsigned short) (F_CPU / (baud * 8) - 1);

	/* Set baud rate */
	UBRR0H = (sv >> 8);
	UBRR0L = sv;
	UCSR0A |= (1 << U2X0);
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

	/* format: asynchronous, 8data, no parity, 1stop bit */
	UCSR0C = (1 << UCSZ01) | (3 << UCSZ00);

	fdevopen(Uart0_PutChar, 0);
}
