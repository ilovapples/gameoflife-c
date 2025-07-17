#ifndef BORDER_H
#define BORDER_H

#include <inttypes.h>
#include <wchar.h>
#include <stdbool.h>

void cp_set_unicode_locale(void);
void get_term_size(uint16_t *rows, uint16_t *cols);
void goxy(uint16_t x, uint16_t y);
void clrscreen(void);

void printbox(uint16_t w, uint16_t h, uint16_t x, uint16_t y, const wchar_t charset[8], bool solid);
#ifndef _WIN32
void printbox_color(uint16_t w, uint16_t h, uint16_t x, uint16_t y, const wchar_t charset[8], 
			  uint8_t fgclr, uint8_t bgclr, bool solid);
#endif

void printgrid(uint16_t grid_w, uint16_t grid_h, uint16_t grid_nx, uint16_t ny, uint16_t x, uint16_t y,
		   const wchar_t charset[13]);

#endif /* BORDER_H */
