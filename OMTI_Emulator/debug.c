/*
 * debug.c
 *
 * Created: 26.02.2022 20:30:32
 *  Author: marek.hummel
 */
#include <stdio.h>
#include <string.h>

#define TET_MIN(a, b)	((a) < (b) ? (a) : (b))

/******************************************************************************/
/* HexPrint */
/******************************************************************************/
void HexPrint(uint8_t *pData, uint16_t Length, uint32_t StartAddr)
{
	char *start, *end;
	char *p1, *p2;
	long len = StartAddr;
	char text[2] = { 0, 0 };

	end = (char*)pData + Length;	/* end is first location not to dump */

	start = (char*)pData;
	do {
		printf("0x%04lx:", len);
		len += 16;
		if (start >= end)
		continue;
		p2 = TET_MIN(start + 16, end);
		for (p1 = start; p1 < p2; p1++)
		printf(" %02x", (int)*p1 & 0xff);
		while (p1++ <= start + 16)
		printf("   ");
		for (p1 = start; p1 < p2; p1++) {
			text[0] = *p1 > '\040' && *p1 < '\177' ? *p1 : '.';
			printf("%s", text);
		}
		printf("\r\n");
	} while ((start += 16) < end);
} 
