#ifndef _SERIAL0_H_
#define _SERIAL0_H_

#include <stdint.h>

void Uart0_Init(uint32_t baud);
int Uart0_PutChar(char c, FILE *fd);
int Uart0_NoBlock_GetChar(void);

#endif	/* _SERIAL0_H_ */
