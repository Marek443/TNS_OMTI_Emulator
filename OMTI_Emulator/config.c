/*
 * config.c
 *
 * Created: 18.03.2022 8:19:07
 *  Author: marek.hummel
 */ 

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "input.h"
#include "FatFs/mmc_avr.h"
#include "FatFs/ff.h"
#include "sd_card.h"
#include "config.h"


// Struktura konfigurace a hodnot systemu
SYS_DATA sys_data;

FIL Fil_CFG;					/* File object needed for each open file */

//***************************************************************************************
// Nacteni parametru konfigurace se souboru CONFIG_FILE ulozeneho na SD karte
//***************************************************************************************
// param_name ... jmeno parametru
// fp         ... ukazatel na soubor konfigurace
// param_name ... jmeno paramatru
// dst        ... cilova pamet pro nactenou hodnotu
// maxlen     ... velikost ciloveho bufferu
// return     ... 1 = OK, 0 pri chybe
//***************************************************************************************
static uint8_t GetCfgParam(FIL *fp, const char *param_name, char *dst, int maxlen)
{
	char *pTmp;
	char buff[256];		/* Pracovni buffer, pozor na velikost stacku */ 

	// Ukazatel na zacatek souboru
	f_lseek(fp, 0);
	
	while(f_eof(&Fil_CFG) == 0) {

		// Nacteni jednoho radku ze souboru
		f_gets(buff, sizeof(buff), &Fil_CFG);
		
		// Komentaze nebrat
		if(*buff == '#' || *buff == '/') continue;

		// Zniceni znaku '\r' a '\n'
		for(pTmp = buff; *pTmp; pTmp++) {
			if((*pTmp == '\r') || (*pTmp == '\n')) {
				*pTmp = 0;
				break;
			}
		}
		
		// Preskoceni mezer a tabulatoru
		for(pTmp = buff; *pTmp && (*pTmp == ' ') && (*pTmp == '\t'); pTmp++);

		// Vyhledani jmena promenne, muzi zacinat znakem a-z nebo A-Z
		if(isalpha(*pTmp)) {

			// Porovnani jmena promenne
			if(!strncmp(pTmp, param_name, strlen(param_name))) {

				// Ziskani pozice '=' v nactenem radku
				pTmp = strchr(pTmp, '=');
				if(!pTmp) {
					printf("GetCfgParam: parameter \"%s\" without value\r\n", param_name);
					return 0;
				}
				// Po preskoceni mezer a tabulatoru za '=' mame hotovo
				while(*pTmp++) {
					if((*pTmp != ' ') || (*pTmp != '\t'))  {
						strncpy(dst, pTmp, maxlen - 1);
						printf("GetCfgParam: %s=%s\r\n", param_name, dst);
						return 1;
					}
				}
			}
		}
	}
	
	printf("GetCfgParam: parameter \"%s\" not found\r\n", param_name);
	return 0;
}


void Config_Init(void)
{
	memset((char*)&sys_data, 0, sizeof(sys_data));
}




//***************************************************************************************
// Ulozeni parametru konfigurace do souboru CONFIG_FILE na SD karte
//***************************************************************************************
// param_name ... jmeno parametru
// value      ... hodnota parametru
// return     ... 0 = OK, CFG_ERROR(0xff) = chyba
//***************************************************************************************
void SaveConfig(void)
{
	// Otevreni souboru konfigurace
	FRESULT fr = f_open (&Fil_CFG, CONFIG_FILE, FA_WRITE | FA_CREATE_ALWAYS );

	printf("SaveConfig, fr = %x\r\n", fr);
	if(fr != FR_OK) return;

	uint8_t i;
	for(i = 0; i < DRV_NUM; i++) {
		f_printf(&Fil_CFG, "filename%u=%s\r\n", i + 1, sys_data.img[i].filename);
	}
	
	f_close(&Fil_CFG);
}


void Config_Proc(uint8_t sd_status)
{
	if((sd_status & SD_STATE_DISK_OK) == 0) {
		// SD karta neni ready, tak rychle pryc
		return; 
	}

	Config_Init();

	// Nacteni konfigurace a inicializace systemu
	printf("Config Load\r\n");

	// Otevreni souboru konfigurace
	FRESULT fr = f_open (&Fil_CFG, CONFIG_FILE, FA_READ | FA_OPEN_EXISTING );
	if(fr != FR_OK) {
		printf("Nepodarilo se otevrit soubor konfigurace \"" CONFIG_FILE "\"\r\n");
		return;
	}

	uint8_t i;
	char val_name[16];
	for(i = 0; i < DRV_NUM; i++) {
		sprintf(val_name, "filename%u", i + 1);
		GetCfgParam(&Fil_CFG, val_name, sys_data.img[i].filename, FILENAME_LEN);

		sd_open_img(i, sys_data.img[i].filename);

	}
	
	f_close(&Fil_CFG);
}


