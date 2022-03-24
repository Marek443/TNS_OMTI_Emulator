/*
 * scsi.c
 *
 * Created: 27.02.2022 20:28:14
 *  Author: marek.hummel
 */

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

#include "board.h"
#include "debug.h"
#include "scsi.h"
#include "sd_card.h"

//#define DBG	1

//***********************************************
// Definice SCSI signalu
//***********************************************
// PA[0..7] - DB[0..7]		In/Out
// PC2      - BSY			Out
// PC3      - RST			In
// PC4      - SEL			In
// PC5      - REQ			Out
// PC6      - ACK			In
// PC7      - PAR			In/Out
// PD4      - C/D			Out
// PD5      - I/O			Out
// PD7      - MSG			Out
//***********************************************


//#define DPRINT(x) printf(x);
#define DB_INPUT	DDRA = 0x00; DB_PORT = 0x00;
#define DB_OUTPUT	DDRA = 0xff;
#define DB_PORT		PORTA
#define DB_PIN		PINA

#define RST_INIT	DDRC &= ~(1 << PC3); PORTC |= (1 << PC3);
#define RST_GET()	(PINC & (1 << PC3))

#define SEL_INIT	DDRC &= ~(1 << PC4); PORTC |= (1 << PC4);
#define SEL_GET()	(PINC & (1 << PC4))

#define ACK_INIT	DDRC &= ~(1 << PC6); PORTC |= (1 << PC6);
#define ACK_GET()	(PINC & (1 << PC6))

#define BSY_INIT	BSY_HI
#define BSY_LO		DDRC |= (1 << PC2); PORTC &= ~(1 << PC2); LED1_ON;
#define BSY_HI		PORTC |= (1 << PC2); DDRC &= ~(1 << PC2); LED1_OFF;

#define REQ_INIT	REQ_HI
#define REQ_LO		DDRC |= (1 << PC5); PORTC &= ~(1 << PC5);
#define REQ_HI		PORTC |= (1 << PC5); DDRC &= ~(1 << PC5);

#define CD_INIT		CD_HI
#define CD_LO		DDRD |= (1 << PD4); PORTD &= ~(1 << PD4);
#define CD_HI		PORTD |= (1 << PD4); DDRD &= ~(1 << PD4);

#define IO_INIT		IO_HI
#define IO_LO		DDRD |= (1 << PD5); PORTD &= ~(1 << PD5);
#define IO_HI		PORTD |= (1 << PD5); DDRD &= ~(1 << PD5);

#define MSG_INIT	MSG_HI
#define MSG_LO		DDRD |= (1 << PD7); PORTD &= ~(1 << PD7);
#define MSG_HI		PORTD |= (1 << PD7); DDRD &= ~(1 << PD7);

#define PAR_INIT	DDRC &= ~(1 << PC7);


uint8_t cmd_buffer[6];
uint8_t data_buffer[10];
uint8_t block_buffer[2048];


//***************************************************************************************
// Cteni dat SASI karty, TNS posila dat do disku
//***************************************************************************************
// 6.9 DATA IN OR OUT PHASE
// Data OUT WRITE, strana 6-7
//***************************************************************************************
// return ... 0 == data OK, cokoli jineho reset
//***************************************************************************************
static uint8_t DataRead(uint8_t *pData, uint16_t num)
{
	while(num--) {

		// Stazeni REQ signalu, to je povel pro SASI kartu ze muze poslat byte
		REQ_LO;	
				
		// Cekam na spadovou hranu ACK, tim poznam ze byte dorazil
		while(ACK_GET()) {
			if(!RST_GET()) {
				// Doslo k resetu zbernice, konec
				return 1;
			}
		}
		
		// Precteni bytu, invertovani a ulozeni do bufferu s posunutim ukazatela
		*pData++ = ~DB_PIN;		
		
		// Huu uz mam byte, reknu to teda SASI karte
		REQ_HI;		

		// Cekam na nabeznou hranu ACK, tim se proces doruceni totoho bytu dokoncen 
		while(!ACK_GET()) {
			if(!RST_GET()) {
				// Doslo k resetu zbernice, konec
				return 1;
			}
		}
	}

	// Hotovo bez chyby
	return 0;
}


//***************************************************************************************
// Odeslani byto do SASI - Zdenkova funkce prdni
//***************************************************************************************
static uint8_t SendByte(uint8_t tx_byte)
{
	// Vystaveni status bytu na zbernici
	DB_PORT = ~tx_byte;
	DB_OUTPUT;
	
	// Stazeni REQ, tim se aktivuje cteni SASI karty
	REQ_LO;
	
	// Cekam na spadovou hranu ACK, tim poznam ze byl byte precteni SASI kartou
	while(ACK_GET()) {
		if(!RST_GET()) {
			// Doslo k resetu zbernice, konec
			return 1;
		}
	}

	// Reakce na ACK od SASI, vytazeni REQ do hi
	REQ_HI;
	DB_INPUT;

	// Cekam na nabeznou hranu ACK, tim se proces odeslani status bytu dokoncen
	while(!ACK_GET()) {
		if(!RST_GET()) {
			// Doslo k resetu zbernice, konec
			return 1;
		}
	}	
	return 0;	
}


uint8_t SendStatAndMsg(uint8_t status_byte, uint8_t message_byte)
{
	// Zbernice do stavu STATUS and MESSAGE prenosu
	CD_LO;
	IO_LO;
	
	// Odeslani status bytu
	if(SendByte(status_byte)) return 1;
	
	
	// A ted odeslame message byte
	MSG_LO;

	if(SendByte(message_byte)) return 1;

	// MSG hotovo
	MSG_HI;
			
	// Uvolneni dratu modu zbernice
	CD_HI;
	IO_HI;

	// Povel je vzdy posledni takze se uvolni i BSY
	BSY_HI;
	
	return 0;	
}

//***************************************************************************************
// Zapis dat do SASI karty, Disk posila data do TNS
//***************************************************************************************
// 6.9 DATA IN OR OUT PHASE
// Data IN Read, strana 6-7
//***************************************************************************************
static uint8_t DataWrite(uint8_t *pData, uint16_t num)
{
	// Zbernice do stavu DATA IN READ
	CD_HI;
	IO_LO;	

	while(num--) {
		cli();
		// Odeslani status bytu
		if(SendByte(*pData++)) return 1;
	}
	return 0;
}



typedef enum {
	SCSI_RESET,
	SCSI_SELECTION,
	SCSI_COMMAND,
	SCSI_COMMAND_DONE
} SCSI_STATE;


void scsi_proc(void)
{
	static SCSI_STATE state = SCSI_RESET;
	static __attribute__((unused)) uint8_t selected_device = 0;
	uint16_t block_size;

fuj:

	// Test RST signalu
	if(!RST_GET()) {
		// Reset je aktivni
		state = SCSI_RESET;
	}

	switch(state) {

	// Reset vseho
	case SCSI_RESET:

		BSY_INIT;
		REQ_INIT;
		CD_INIT;
		IO_INIT;
		MSG_INIT;

		RST_INIT;
		SEL_INIT;
		ACK_INIT;
		PAR_INIT;

		DB_INPUT;
		
		selected_device = 0;
			
		if(RST_GET()) {
			state = SCSI_SELECTION;	
		}
		break;

	case SCSI_SELECTION:
	
		sei();
	
		// Cekam na spadovou hranu signalu SEL
		if(SEL_GET()) break;

		cli();
		
		// Huraa spadova hrana SEL uz je tu, precteni PORTu
		// Zde je id selectovaneho zarizeni
		selected_device = ~DB_PIN;
			
		BSY_LO;

		state = SCSI_COMMAND;
		break;

	case SCSI_COMMAND:
	
		// Cekam na nabezno hranu signalu SEL
		if(!SEL_GET()) break;		
	
		// Nastavenim CD do low to aktivuje doruceni command povelu
		CD_LO;
			
		// Precetni command povelu, ten ma pevnou delku 6 bytu
//		cli();
		if(DataRead(cmd_buffer, 6)) {
			// Cteni se nepovedlo, konec
			state = SCSI_RESET;
			break;	
		}
//		sei();
						
		// Command je tu nahozeni CD
		CD_HI;
			
		state = SCSI_COMMAND_DONE;
		break;
		
	case SCSI_COMMAND_DONE:

#ifdef DBG
		printf("\r\n***********************************************\r\n");
		printf("selected_device = %d\r\n", selected_device);
		printf("cmd = 0x%02x\r\n", CMD_GET_CMD(cmd_buffer));
		printf("lun = %u\r\n", CMD_GET_LUN(cmd_buffer));
		HexPrint(cmd_buffer, 6, 0);
#endif
		if(cmd_buffer[0] == 0xc2) {
			// Precetni dat povelu C2
//			cli();
			if(DataRead(data_buffer, 10)) {
				// Cteni se nepovedlo, konec
				state = SCSI_RESET;
				break;
			}
			SendStatAndMsg(cmd_buffer[1] & 0x60, 0);
//			sei();			

#ifdef DBG
			HexPrint(data_buffer, 10, 0);

			if(data_buffer[7] & 0x80) {
				// When bit 7 of byte 8 of the Parameter List is set to one, the
				// following Flexible Disk Parameter List, allows connection of
				// either 5 1/4" or 8"
				printf("Flexible Disk, floppy 5 1/4\" or 8\"\r\n");
				printf("cyl_addr = %u\r\n", CMD_C2_CYL_ADDR_FLOPPY(data_buffer));
				
			} else {
				// When bit 7 of byte 8 is set to zero, the following list allows
				// connection of any, and different, ST506/412 compatible disk
				// drives to the two Winchester ports of the controller.

				printf("Winchester disk\r\n");
				printf("heads = %u\r\n", CMD_C2_HEADS(data_buffer));
				printf("cyl_addr = %u\r\n", CMD_C2_CYL_ADDR(data_buffer));
				printf("type = %u\r\n",CMD_C2_CYL_TYPE(data_buffer));
				printf("cyl per track = %u\r\n", CMD_C2_CYL_SEC_PER_TRACK(data_buffer));
			}

			printf("Return status = 0x%02x, message = 0x%02x\r\n", cmd_buffer[1] & 0x60, 0);

#endif			
		}
		
		// 7.7.1 DEFINE FLEXIBLE DISK FORMAT Command (HEX CO)
		else if(cmd_buffer[0] == 0xc0) {

#ifdef DBG
			printf("Flexible Disk Format\r\n");
			printf("cyl per track = %u\r\n", cmd_buffer[4]);
			printf("fdd track format = 0x%02x, viz doc strana 7-25,26\r\n", cmd_buffer[5]);
			printf("Return status = 0x%02x, message = 0x%02x\r\n", cmd_buffer[1] & 0x60, 0);
#endif	

//			cli();
			SendStatAndMsg(cmd_buffer[1] & 0x60, 0);
//			sei();
					
		}
		
		// 7.5.10 SEEK Command (HEX OB)
		else if(cmd_buffer[0] == 0x0b) {

#ifdef DBG
			printf("SEEK Command\r\n");
			printf("lba = %lu\r\n", CMD_READ_GET_LBA(cmd_buffer));
#endif
//			cli();
			SendStatAndMsg(cmd_buffer[1] & 0x60, 0x00);
//			sei();
			
		}

		// 7.8.3 READ IDENTIFIER Command (HEX E2)
		else if(cmd_buffer[0] == 0xe2) {

			uint32_t block = CMD_READ_GET_LBA(cmd_buffer) / 512;
			
			data_buffer[0] = 0;
			data_buffer[1] = (block / 9) + 1;		// Stopa  1 - 40
			data_buffer[2] = block & 1;			// Hlava  0 - 1
			data_buffer[3] = block % 9;			// Sector 0 - 8

#ifdef DBG
			printf("READ IDENTIFIER Command\r\n");
			printf("block = %lu\r\n", block);
			printf("stopa  (1-40) = %u\r\n", data_buffer[1]);
			printf("hlava  (0-1)  = %u\r\n", data_buffer[2]);
			printf("sector (0-8)  = %u\r\n", data_buffer[3]);
#endif

//			cli();
			DataWrite(data_buffer, 4);
			SendStatAndMsg(cmd_buffer[1] & 0x60, 0x00);
//			sei();

			
		}
									
		// 8.3.6 READ Command (HEX 08) (Sequential Mode only)
		else if(cmd_buffer[0] == 0x08) {
			uint8_t block_count = CMD_READ_GET_BLOCK(cmd_buffer);
#ifdef DBG
			printf("lba = %lu\r\n", CMD_READ_GET_LBA(cmd_buffer));
			printf("block_count = %u\r\n", block_count);
			printf("control = 0x02%x\r\n", CMD_READ_GET_CONTROL(cmd_buffer));
			printf("Return status = 0x%02x, message = 0x%02x\r\n", cmd_buffer[1] & 0x60, 0);
#endif			
			if(CMD_GET_LUN(cmd_buffer) < 2) {
				// HDD
				block_size = 256;
			} else {
				// FDD
				block_size = 512;				
			}

			uint32_t addr = CMD_READ_GET_LBA(cmd_buffer) * block_size;
			while(block_count--) {
				sd_read(CMD_GET_LUN(cmd_buffer), addr, block_buffer, block_size);
				DataWrite(block_buffer, block_size);
				addr += block_size; 
			}
			
			SendStatAndMsg(cmd_buffer[1] & 0x60, 0x00);
		} 

		// 8.3.7 WRITE Command (HEX OA)
		else if(cmd_buffer[0] == 0x0A) {
			uint8_t block_count = CMD_READ_GET_BLOCK(cmd_buffer);
#ifdef DBG
			printf("lba = %lu\r\n", CMD_READ_GET_LBA(cmd_buffer));
			printf("block_count = %u\r\n", block_count);
			printf("control = 0x02%x\r\n", CMD_READ_GET_CONTROL(cmd_buffer));
			printf("Return status = 0x%02x, message = 0x%02x\r\n", cmd_buffer[1] & 0x60, 0);
#endif
			if(CMD_GET_LUN(cmd_buffer) < 2) {
				// HDD
				block_size = 256;
			} else {
				// FDD
				block_size = 512;
			}

			uint32_t addr = CMD_READ_GET_LBA(cmd_buffer) * block_size;
			while(block_count--) {
				DataRead(block_buffer, block_size);
				sd_write(CMD_GET_LUN(cmd_buffer), addr, block_buffer, block_size);
				addr += block_size;
			}
			sd_sync(CMD_GET_LUN(cmd_buffer));

			SendStatAndMsg(cmd_buffer[1] & 0x60, 0x00);
			
		}

		state = SCSI_RESET;		
		break;
	}

	if(state != SCSI_SELECTION) goto fuj;
}
