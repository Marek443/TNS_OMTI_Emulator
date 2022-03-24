/*
 * sd_card.c
 *
 * Created: 05.03.2022 20:14:00
 *  Author: marek.hummel
 */
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "FatFs/mmc_avr.h"
#include "sd_card.h"
#include "config.h"
#include "debug.h"
#include "board.h"

FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */


//FIL Fil_LUN[4];


/*
const char *filename1 = "hdddecin.img";
const char *filename2 = "SCWPV.IMG";
//const char *filename2 = "TNS2.IMG";
const char *filename3 = "FDD_1.IMG";
const char *filename4 = "FDD_2.IMG";
*/

static uint8_t state = SD_STATE_NO_DISK;



void sd_open_img(uint8_t lun, const char *filename)
{
	FRESULT fr;
	if(lun >= 4) return;
	fr = f_open(&sys_data.img[lun].fd, filename, FA_READ | FA_WRITE | FA_OPEN_EXISTING );
	printf("Open file %s, rc = %d, file size = %lu, LUN = %u\r\n", filename, fr, (uint32_t)f_size (&sys_data.img[lun].fd), lun);
}


uint8_t sd_read(uint8_t lun, uint32_t addr, uint8_t *buffer, int len)
{
	f_lseek(&sys_data.img[lun].fd, addr);
	UINT br;
	f_read(&sys_data.img[lun].fd, buffer, len, &br);

//	printf("sd_read, addr = %lu(0x%lx), len = %d\r\n", addr, addr, len);
//	HexPrint(buffer, len, addr);
	
	return 0; 
}


uint8_t sd_write(uint8_t lun, uint32_t addr, uint8_t *buffer, int len)
{
//LED1_ON;
	f_lseek(&sys_data.img[lun].fd, addr);
	UINT br;
	f_write(&sys_data.img[lun].fd, buffer, len, &br);

//	printf("sd_write, addr = %lu(0x%lx), len = %d\r\n", addr, addr, len);
//	HexPrint(buffer, len, addr);
	
	return 0;	
}


//***************************************************************************************
// Flush cached data of the writing file
//***************************************************************************************
void sd_sync(uint8_t lun)
{
	f_sync(&sys_data.img[lun].fd);
}


uint8_t sd_card_proc(void)
{
	static DSTATUS sd_status_last = 0xff;
	DSTATUS sd_status = mmc_disk_status() & STA_NODISK;
	
	if(sd_status == sd_status_last) return state;

	uint8_t tmp_state = state;

	sd_status_last = sd_status;
	sd_status = mmc_disk_initialize();
	
	if (sd_status & STA_NODISK) {
		printf("SD Card off\r\n");
		state = SD_STATE_NO_DISK;
		return state | ((state != tmp_state) ? SD_STATE_CHANGE : 0);
	}
	
//	mmc_disk_initialize();	

//	FRESULT fr;
//	DIR dj;
//	FILINFO fno;

	FRESULT fr = f_mount(&FatFs, "0:", 1);		/* Give a work area to the default drive */
	
	if (fr == FR_OK) {
		printf("Mount successful\r\n");
		state = SD_STATE_DISK_OK;
	} else {
		printf("Mount failed (%d)\r\n", fr);
		state = SD_STATE_DISK_ERROR;
	}

	return state | ((state != tmp_state) ? SD_STATE_CHANGE : 0);

}
#if 0
	printf("\r\nListing files\r\n\t.... \r\n");
	_delay_ms(100);
	
	fr = f_findfirst(&dj, &fno, "", "*"); /* Start to search for all files */

	while (fr == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
		if (fno.fattrib & AM_DIR )
		printf("%s\\\r\n", fno.fname);	// Print directory name
		else
		printf("%s\r\n", fno.fname);                /* Print the file name */
		fr = f_findnext(&dj, &fno);               /* Search for next item */
	}
	printf("\t....\r\nListing files done.\r\n");

	f_closedir(&dj);

	sd_open_img(0, filename1);
	sd_open_img(1, filename2);
	sd_open_img(2, filename3);
	sd_open_img(3, filename4);


//	f_unmount("0:");
//	_delay_ms(1000);
}
#endif

uint8_t sd_card_state(void)
{
	return state;	
	
}


