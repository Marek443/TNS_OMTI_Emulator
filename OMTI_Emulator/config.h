/*
 * config.h
 *
 * Created: 18.03.2022 8:19:18
 *  Author: marek.hummel
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include "FatFs/mmc_avr.h"
#include "sd_card.h"

#define CONFIG_FILE		"config.cfg"		/* Jmeno souboru konfigurace */

#define DRV_NUM			4		/* Pocet emulovanych jednotek */
#define FILENAME_LEN	128

typedef struct {
	struct {
		char filename[FILENAME_LEN];
		FIL fd;
	} img[DRV_NUM];
	
} SYS_DATA;

extern SYS_DATA sys_data;

struct opt {
	const char *name;
};

struct opts {
	FIL *file;
	const struct opt *opts;
	char *arg;
	int argmax;
};

int get_next_opt(struct opts *opts);
#define OPT_eof -1
#define OPT_section -2

void SaveConfig(void);
#define CFG_ERROR	0xff


void Config_Init(void);
void Config_Proc(uint8_t sd_status);

#endif /* CONFIG_H_ */
