/* Used for converting font files to a kind-of-boolean
 * array that can be emdedded in C/C++ source code.
 */

#include <stdint.h>
#include <stdio.h>

/* Change for the desired font. */
#define FONT_FILENAME "VGA-ROM.F08"
#define CHAR_HEIGHT_IN_PIX 8
/* Char width should always be 8 pixels; 8 bits per row in font file. */

int main() {
	uint8_t column[CHAR_HEIGHT_IN_PIX], *rowptr;
	int rownum, rowmask;
	FILE *fp = fopen(FONT_FILENAME, "rb");
	if (!fp) {
		return 1;
	}
	while (!feof(fp)) {
		fread(&column, CHAR_HEIGHT_IN_PIX, 1, fp);
		for (rownum=0, rowptr=column; rownum<CHAR_HEIGHT_IN_PIX; rownum++, rowptr++) {
			printf("\n");
			for (rowmask=7; rowmask>=0; rowmask--) {
				if (((*rowptr)&(1<<rowmask))>>rowmask) {
					printf("1,");
				} else {
					printf("0,");
				}
			}
		}
		printf("\n");
	}
	fclose(fp);
	return 0;
}
