/*
 * sd_card.h
 *
 * Created: 05.03.2022 20:14:30
 *  Author: marek.hummel
 */ 


#ifndef SD_CARD_H_
#define SD_CARD_H_

void sd_open_img(uint8_t lun, const char *filename);

uint8_t sd_card_proc(void);
uint8_t sd_read(uint8_t lun, uint32_t addr, uint8_t *buffer, int len);
uint8_t sd_write(uint8_t lun, uint32_t addr, uint8_t *buffer, int len);
void sd_sync(uint8_t lun);


uint8_t sd_card_state(void);
	#define SD_STATE_NO_DISK		1
	#define SD_STATE_DISK_OK		2
	#define SD_STATE_DISK_ERROR		4
	#define SD_STATE_CHANGE			8


#endif /* SD_CARD_H_ */