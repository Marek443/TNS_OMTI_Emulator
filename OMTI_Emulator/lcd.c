#include <stdio.h>
#include <string.h>
#include <u8g.h>
#include "board.h"
#include "sd_card.h"
#include "FatFs/mmc_avr.h"
#include "Input.h"

/*
 * lcd.c
 *
 * Created: 10.03.2022 21:45:58
 *  Author: marek.hummel
 */


#define VER "0.5"

u8g_t u8g;

typedef enum {
	PAGE_START = 0,
	PAGE_CD_CARD_STATE,
	PAGE_BASE
} LCD_PAGE;

typedef struct {
	uint8_t update;
	LCD_PAGE page;	
} LCD;

static LCD lcd;

static uint8_t animate = 0;
#define ANIMATE_TOP	80

void lcd_init(void)
{
	u8g_InitI2C(&u8g, &u8g_dev_ssd1306_128x32_i2c, U8G_I2C_OPT_NONE);

	u8g_SetContrast(&u8g, 0);

	memset((char*)&lcd, 0, sizeof(lcd));
	lcd.page = PAGE_CD_CARD_STATE;
	lcd.update = 1;
}

static volatile uint8_t lcd_timeout = 1;

	
void lcd_timer_10mS(void)
{
	if(lcd_timeout > 1) {
		lcd_timeout--;	
	}
}


// obrazek prekonvertovany do XBM formatu
// treba tady: https://www.online-utility.org/image/convert/to/XBM
const uint8_t sd_card_img_19x24[] = {
  0xFF, 0xFF, 0x07, 0x01, 0x00, 0x04, 0x01, 0x00, 0x04, 0xF1, 0x7C, 0x04,
  0x99, 0xCD, 0x04, 0x19, 0xCC, 0x04, 0x79, 0xCC, 0x04, 0xE1, 0xCD, 0x04,
  0x81, 0xCD, 0x04, 0x99, 0xCD, 0x04, 0xF1, 0x7C, 0x04, 0x01, 0x00, 0x06,
  0x03, 0x00, 0x02, 0x02, 0x00, 0x02, 0x03, 0x00, 0x06, 0x01, 0x00, 0x06,
  0x01, 0x00, 0x04, 0x05, 0x00, 0x04, 0x55, 0x55, 0x05, 0x55, 0x55, 0x05,
  0x55, 0x55, 0x05, 0x52, 0x55, 0x05, 0x04, 0x00, 0x04, 0xF8, 0xFF, 0x03,
};


const uint8_t fire_img_28x28[4][112] = {
	{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x01, 0x07,
	0x00, 0x20, 0x01, 0x06, 0x00, 0x30, 0x03, 0x06, 0x00, 0x30, 0x03, 0x02,
	0x00, 0x70, 0x03, 0x00, 0x00, 0xE0, 0x23, 0x00, 0x00, 0x60, 0x3B, 0x00,
	0x00, 0x30, 0x1F, 0x00, 0x00, 0x18, 0x1D, 0x00, 0x00, 0x6E, 0x1D, 0x00,
	0x00, 0xEF, 0x1B, 0x00, 0x00, 0xDF, 0x13, 0x00, 0x00, 0xFF, 0x0F, 0x00,
	0x80, 0xFF, 0x0F, 0x00, 0x80, 0xFE, 0x1F, 0x00, 0xC0, 0xFE, 0x3F, 0x00,
	0xC0, 0xFF, 0x3F, 0x00, 0xE0, 0xFF, 0x3F, 0x00, 0xC0, 0xFF, 0x5F, 0x00,
	0xC0, 0xFF, 0x4F, 0x00, 0x00, 0xFE, 0x6F, 0x00, 0x00, 0xFC, 0x7F, 0x00,
	0x00, 0xFC, 0x3F, 0x00 },

	{
	0x00, 0xC0, 0x01, 0x06, 0x00, 0x80, 0x87, 0x03, 0x00, 0x00, 0x0F, 0x01,
	0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x26, 0x00,
	0x00, 0x08, 0x42, 0x00, 0x00, 0x18, 0x40, 0x00, 0x00, 0x38, 0x61, 0x00,
	0x00, 0x38, 0x63, 0x00, 0x00, 0x58, 0x33, 0x00, 0x00, 0xF4, 0x13, 0x00,
	0x00, 0xF0, 0x03, 0x00, 0x00, 0xF8, 0x03, 0x00, 0xC0, 0xFB, 0x01, 0x00,
	0xE0, 0xE9, 0x11, 0x00, 0xF0, 0xE1, 0x3F, 0x00, 0xF0, 0xF1, 0x1F, 0x00,
	0xC0, 0xF1, 0x4F, 0x00, 0xE0, 0xFF, 0x63, 0x00, 0xC0, 0xFF, 0x63, 0x00,
	0xC0, 0xFF, 0xF7, 0x00, 0x80, 0xFF, 0xFF, 0x01, 0x80, 0xFF, 0xDF, 0x01,
	0x00, 0xFF, 0x9F, 0x01, 0x00, 0xFE, 0x9F, 0x01, 0x00, 0xFE, 0x5F, 0x00,
	0x00, 0xFE, 0x2F, 0x00 },

	{
	0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x72, 0x00,
	0x00, 0x40, 0xF3, 0x00, 0x00, 0x20, 0xB3, 0x00, 0x00, 0xE0, 0x63, 0x00,
	0x00, 0xE6, 0x6B, 0x00, 0x00, 0xEF, 0x6D, 0x00, 0x00, 0xEF, 0x2D, 0x00,
	0x00, 0xCF, 0x2F, 0x00, 0x80, 0xCF, 0x1F, 0x00, 0x80, 0xC7, 0x07, 0x00,
	0x80, 0xC3, 0x33, 0x00, 0x80, 0xE1, 0x33, 0x00, 0xC0, 0xE1, 0x3F, 0x00,
	0xC0, 0xF1, 0x3F, 0x02, 0xC0, 0xFF, 0x3F, 0x02, 0xC0, 0xFF, 0x1F, 0x03,
	0xC0, 0xFF, 0x9F, 0x03, 0xC0, 0xFF, 0xCF, 0x03, 0xC0, 0xFF, 0xEB, 0x01,
	0xC0, 0xFF, 0xC7, 0x00, 0xC0, 0xFF, 0x8F, 0x01, 0x80, 0xFF, 0x9F, 0x01,
	0x80, 0xFF, 0x5F, 0x00, 0x00, 0xFF, 0x1F, 0x00, 0x00, 0xFE, 0x1F, 0x00,
	0x00, 0xFE, 0x1F, 0x00 },
	{
	0x00, 0x06, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00,
	0x00, 0xF0, 0x00, 0x00, 0x00, 0xF0, 0x01, 0x00, 0x00, 0xF0, 0x01, 0x00,
	0x00, 0x30, 0x01, 0x00, 0x00, 0x10, 0x06, 0x00, 0x00, 0x08, 0x1E, 0x00,
	0x00, 0x08, 0x1E, 0x00, 0x00, 0x00, 0x1C, 0x03, 0x00, 0x00, 0x9C, 0x07,
	0x00, 0x00, 0x1C, 0x0F, 0x00, 0x40, 0x1E, 0x0F, 0x00, 0x38, 0x0F, 0x0C,
	0x00, 0x9E, 0x03, 0x08, 0x00, 0xCE, 0x01, 0x00, 0x00, 0xEF, 0x01, 0x00,
	0x00, 0xFF, 0x03, 0x00, 0x00, 0xFE, 0x1F, 0x00, 0x00, 0xFC, 0x1F, 0x00,
	0x00, 0xF8, 0x1F, 0x00, 0x00, 0xFF, 0x1F, 0x00, 0x00, 0xFF, 0x1F, 0x00,
	0x00, 0xFF, 0x0F, 0x00, 0x00, 0xFF, 0x03, 0x00, 0x00, 0xFE, 0x0F, 0x00,
	0x00, 0xF8, 0x1F, 0x00 }
	};


const uint8_t ok_img_28x28[] = {
	0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x80, 0x0F, 0x00, 0x00, 0xC0, 0x07,
	0x00, 0x00, 0xE0, 0x03, 0x00, 0x00, 0xF0, 0x01, 0x00, 0x00, 0xF8, 0x00,
	0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3F, 0x00,
	0x00, 0x80, 0x1F, 0x00, 0x00, 0xC0, 0x0F, 0x00, 0x00, 0xE0, 0x07, 0x00,
	0x04, 0xF0, 0x07, 0x00, 0x3E, 0xF8, 0x03, 0x00, 0xFE, 0xFC, 0x01, 0x00,
	0xFC, 0xFF, 0x01, 0x00, 0xF8, 0xFF, 0x00, 0x00, 0xF0, 0x7F, 0x00, 0x00,
	0xF0, 0x7F, 0x00, 0x00, 0xE0, 0x3F, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00,
	0x80, 0x1F, 0x00, 0x00, 0x80, 0x1F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
	0x00, 0x0F, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, };


const uint8_t ok_img_16x16[] = {
	0x00, 0xC0, 0x00, 0xF0, 0x00, 0x38, 0x00, 0x3C, 0x00, 0x1E, 0x00, 0x0F,
	0x80, 0x07, 0x8E, 0x03, 0xFE, 0x03, 0xFC, 0x01, 0xF8, 0x01, 0xF0, 0x00,
	0xF0, 0x00, 0x60, 0x00, 0x60, 0x00, 0x00, 0x00, };

void lcd_generator(void)
{
	switch(lcd.page) {
		case PAGE_START:
			u8g_SetFont(&u8g, u8g_font_9x18);
			u8g_DrawStr(&u8g, 5, 15, "OMTI Emulator");
			u8g_DrawStr(&u8g, 22, 32, "Verze " VER);
			break;
		case PAGE_CD_CARD_STATE:

			break;
		case PAGE_BASE:
			break;
	}
}

//***************************************************************************************
// Obrazovka pri nezastrcene DS karte
//***************************************************************************************
void page_no_sd_card(uint8_t sd_state)
{
	uint8_t pos_y = 0;

	// Zakruhovany citac animace
	if(++animate == ANIMATE_TOP) animate = 0;
				
	// Prekresleni obsahu lcd
	u8g_SetFont(&u8g, u8g_font_9x18);
	u8g_FirstPage(&u8g);
	do {

		u8g_DrawStr(&u8g, 5, 32,  "SD Card");

		// Patice SD karty
		u8g_DrawLine(&u8g, 95, 30, 123, 30);
		u8g_DrawLine(&u8g, 95, 31, 123, 31);

		// Vypocet pozice zasouvani sd karty
		if(animate <= 20) pos_y = 2;
		else if(animate >= 46) pos_y = 27;
		else pos_y = animate - 18;

		// SD karta
		u8g_DrawXBM(&u8g, 100, pos_y, 19, 24, sd_card_img_19x24);		

		if(sd_state == SD_STATE_NO_DISK) {
			// Text pri nezasunute SD karte
			u8g_DrawStr(&u8g, 10, 15, "Insert");
			
		} else {
			// Text pri vadne SD karte
			u8g_DrawStr(&u8g, 10, 15, "Wrong");

			// Animace ohne
			if(pos_y == 27) {
				u8g_DrawXBM(&u8g, 93, 0, 28, 28, fire_img_28x28[animate & 3]);

				// Animace zustane nakonci, opakuje posledni 4 pozice, SD karta navzdy hori
				if(animate == 79) animate = 79 - 4;
			}
		}

	} while(u8g_NextPage(&u8g));	
}


uint8_t page_sd_card_ok(void)
{
	uint8_t pos_y;

	// Citac animace
	if(animate < ANIMATE_TOP) { 
		animate++;
	} else {
		// Animace OK SD karty dobehla, takze konec
		return 1;
	}
	
	// Prekresleni obsahu lcd
	u8g_SetFont(&u8g, u8g_font_9x18);
	u8g_FirstPage(&u8g);
	do {
		u8g_DrawStr(&u8g, 5, 15, "SD Card");
		u8g_DrawLine(&u8g, 95, 30, 123, 30);
		u8g_DrawLine(&u8g, 95, 31, 123, 31);
		
		if(animate <= 20) pos_y = 2;
		else if(animate >= 46) pos_y = 27;
		else pos_y = animate - 18;
		
		u8g_DrawXBM(&u8g, 100, pos_y, 19, 24, sd_card_img_19x24);
	
		if(pos_y == 27) {
			u8g_DrawStr(&u8g, 15, 32,  "Ready");
			u8g_DrawXBM(&u8g, 100, 4, 16, 16, ok_img_16x16);					
		}
		
	} while(u8g_NextPage(&u8g));
	
	return 0;
}




void file_select(uint8_t index)
{
	static uint8_t index_last = 0xff;
	if(index_last == index) return;
	index_last = index;
	
	FRESULT fr;
	DIR dj;
	FILINFO fno;	

	fr = f_findfirst(&dj, &fno, "", "*"); /* Start to search for all files */

	printf("index = %u\r\n", index);

	uint8_t file_index = 0;
	while (fr == FR_OK && fno.fname[0] && (index != file_index)) {         /* Repeat while an item is found */
		if (fno.fattrib & AM_DIR ) {
//		printf("%s\\\r\n", fno.fname);	// Print directory name
		} else {
//		printf("%s\r\n", fno.fname);                /* Print the file name */
			fr = f_findnext(&dj, &fno);               /* Search for next item */
		}
		file_index++;
	}
	

	// Prekresleni obsahu lcd
	u8g_SetFont(&u8g, u8g_font_9x18);
	u8g_FirstPage(&u8g);
	do {
		u8g_DrawStr(&u8g, 5, 15, fno.fname);
		
	} while(u8g_NextPage(&u8g));

	f_closedir(&dj);

}

void lcd_proc(BUTTONS buttons)
{
	uint8_t sd_state = sd_card_state();
	static uint8_t last_sd_state;
	
	// Reset pozice animace SD karty pri zmene stavu
	if(last_sd_state != sd_state) {
		last_sd_state = sd_state;
		animate = 0;
	}
	
	if(sd_state == SD_STATE_DISK_OK) {

		if(page_sd_card_ok() == 0) {
			// SD karta je OK, ale jeste se ceka na dokonceni animace
			return;
		}
					
	} else {
		// Zobrazeni pri problemu s SD kartou
		page_no_sd_card(sd_state);
		return;	
	}
	
	
	static uint8_t file_index = 0;


	if(buttons.change & BUTTON_SW1) {
		if(buttons.value & BUTTON_SW1)	file_index--;
	}

	if(buttons.change & BUTTON_SW2) {
		if(buttons.value & BUTTON_SW2)	file_index++;
	}

	
	file_select(file_index);	
	
	
}

