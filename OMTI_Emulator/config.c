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


static char buf[256];

FIL Fil_CFG;			/* File object needed for each open file */


//***************************************************************************************
// Nacteni parametru konfigurace se souboru CONFIG_FILE ulozeneho na SD karte
//***************************************************************************************
// param_name ... jmeno parametru
// fp         ... ukazatel na soubor konfigurace
// param_name ... jmeno paramatru
// buff       ... pracovni buffer
// len        ... velikost pracovniho bufferu
// return     ... ukazatel na hodnotu hledaneho parametru, 0 pri chybe
//***************************************************************************************
char *GetCfgParam(FIL *fp, const char *param_name, char *buff, int len)
{
	char *pTmp;

	// Ukazatel na zacatek souboru
	f_lseek(fp, 0);
	
	while(f_eof(&Fil_CFG) == 0) {

		// Nacteni jednoho radku ze souboru
		f_gets(buff, len, &Fil_CFG);
		
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
				if(!pTmp) return 0;

				// Po preskoceni mezer a tabulatoru za '=' mame hotovo
				while(*pTmp++) {
					if((*pTmp != ' ') || (*pTmp != '\t'))  return pTmp;
				}
			}
		}
	}
	return 0;
}


//***************************************************************************************
// Ulozeni parametru konfigurace do souboru CONFIG_FILE na SD karte
//***************************************************************************************
// param_name ... jmeno parametru
// value      ... hodnota parametru
// return     ... 0 = OK, CFG_ERROR(0xff) = chyba
//***************************************************************************************
uint8_t SaveConfig(const char *param_name, char *value)
{
	return 0;	
}


void cfg_proc(BUTTONS buttons)
{
	if(buttons.change & BUTTON_SW1) {
		if(buttons.value & BUTTON_SW1)	{
			
	
		FRESULT fr;
		char *pValue;
		
		// Otevreni souboru konfigurace
		fr = f_open (&Fil_CFG, CONFIG_FILE, FA_READ | FA_OPEN_EXISTING );
		if(fr != FR_OK) return;
	
	
		pValue = GetCfgParam(&Fil_CFG, "rpm", buf, sizeof(buf));
		printf("rpm = %s\r\n", pValue ? pValue : "");
	
		pValue = GetCfgParam(&Fil_CFG, "mode", buf, sizeof(buf));
		printf("mode = %s\r\n", pValue ? pValue : "");

		pValue = GetCfgParam(&Fil_CFG, "heads", buf, sizeof(buf));
		printf("heads = %s\r\n", pValue ? pValue : "");
		
		f_close(&Fil_CFG);
			
		}
		
		
	}	
	
}


