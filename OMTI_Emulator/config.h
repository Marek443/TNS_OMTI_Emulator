/*
 * config.h
 *
 * Created: 18.03.2022 8:19:18
 *  Author: marek.hummel
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#define CONFIG_FILE		"config.cfg"		/* Jmeno souboru konfigurace */


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


uint8_t LoadConfig(const char *param_name, char *value, uint8_t maxlen);
uint8_t SaveConfig(const char *param_name, char *value);
#define CFG_ERROR	0xff

#endif /* CONFIG_H_ */
