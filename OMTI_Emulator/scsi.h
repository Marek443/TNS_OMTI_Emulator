/*
 * scsi.h
 *
 * Created: 27.02.2022 20:28:35
 *  Author: marek.hummel
 */ 


#ifndef SCSI_H_
#define SCSI_H_


void scsi_proc(void);


//***************************************************************************************
// Makra pro pristup k hodnotam commandu
//***************************************************************************************
#define CMD_GET_CMD(buffer) buffer[0]
#define CMD_GET_LUN(buffer) ((buffer[1] >> 5) & 3)
#define CMD_READ_GET_LBA(buffer) ((((uint32_t)(buffer[1] & 0x1f)) << 16) | (buffer[2] << 8) | buffer[3])
#define CMD_READ_GET_BLOCK(buffer) buffer[4]
#define CMD_READ_GET_CONTROL(buffer) buffer[5]

#define CMD_C2_HEADS(buffer) (buffer[3] + 1)
#define CMD_C2_CYL_ADDR(buffer) (((buffer[4] << 8) | buffer[5]) + 1) 
#define CMD_C2_CYL_TYPE(buffer) ((buffer[7] >> 4) & 3)
#define CMD_C2_CYL_SEC_PER_TRACK(buffer) (buffer[8] + 1)

#define CMD_C2_CYL_ADDR_FLOPPY(buffer) (buffer[2] + 1) 


#endif /* SCSI_H_ */