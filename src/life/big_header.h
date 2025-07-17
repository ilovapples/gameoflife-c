#ifndef BIG_HEADER_H
#define BIG_HEADER_H

#include <stdint.h>
#include <wchar.h>

#define GETBIT(v, i) ((v>>(7-i)) & 1)
#define SETBIT(v, i) (v |= 1<<(7-i))

#define HELP_MENU_WIDTH  30
#define HELP_MENU_HEIGHT 7

extern wchar_t BASIC_BORDERCHARS[8],
	 	   INVIS_BORDERCHARS[8],
		   W1T1_BORDERCHARS[8],
		   W1T2_BORDERCHARS[8],
		   W1T3_BORDERCHARS[8],
		   W2T1_BORDERCHARS[8];

void set_dims(uint16_t rows, uint16_t cols);

#endif /* BIG_HEADER_H */
