#include <stdio.h>
#include <stdint.h>
#include <locale.h>
#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/ioctl.h>
#endif

typedef uint_fast8_t ubyte;
typedef uint_fast16_t uhex;

#include "border_chars.c"

void cp_set_unicode_locale() {
	#ifdef _WIN32
		setlocale(LC_ALL, ".UTF-8");
		SetConsoleOutputCP(CP_UTF8);
	#else
		setlocale(LC_ALL, "");
	#endif
}

void get_term_size(uhex* rows, uhex* cols) {
	#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		*rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		*cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	} else {
		*rows = 80;
		*cols = 25; // i guess this is the default size?
	}
	#else
	struct winsize size;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1) {
		perror("ioctl");
		return;
	}
	*rows = (uhex) size.ws_row;
	*cols = (uhex) size.ws_col;
	#endif
}

void goxy(uhex x, uhex y) {
	#ifdef _WIN32
		printf("\x1B[%u;%uf", y+1, x+1);
	#else
		printf("\x1B[%lu;%luf", y+1, x+1);
	#endif
}
void clrscreen() {
	printf("\x1B[2J");
}

// separate implementations are defined for printbox because apparently using the windows api on Windows is better than plain old wprintf
#ifdef _WIN32
void printbox(
	uhex w,
	uhex h,
	uhex x,
	uhex y,
	const wchar_t charset[8],
	int solid
) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		printf("FAILED TO OBTAIN CONSOLE HANDLE (windows.h)");
		return;
	}
	
	COORD row_Size = { w, 1 };
	COORD side_Size = { 1, 1 };
	COORD upper_left_Coord = { 0,0 };

	// top bit
	SMALL_RECT top_Region = { x,y, w+x,y };
	CHAR_INFO *top_Buffer = (CHAR_INFO*) calloc(w, sizeof(CHAR_INFO));
	top_Buffer[0].Char.UnicodeChar = charset[4];
	top_Buffer[0].Attributes = 0b0111; // white text
	for (uhex c=1; c <= (w-2); ++c) {
		top_Buffer[c].Char.UnicodeChar = charset[0];
		top_Buffer[c].Attributes = 0b0111;
	}
	top_Buffer[w-1].Char.UnicodeChar = charset[5];
	top_Buffer[w-1].Attributes = 0b0111;
	WriteConsoleOutputW(
			hConsole, 
			top_Buffer, 
			row_Size,
			upper_left_Coord,
			&top_Region);
	free(top_Buffer);
	
	// middle bits
	for (uhex r=1; r<=(h-2); ++r) {
		if (solid) {
			SMALL_RECT curRow_Region = { x,r+y, w+x,r+y };
			CHAR_INFO *curRow_Buffer = (CHAR_INFO*) calloc(w, sizeof(CHAR_INFO));
			curRow_Buffer[0].Char.UnicodeChar = charset[3];
			curRow_Buffer[0].Attributes = 0b0111;
			for (uhex c=1; c <= (w-2); ++c) {
				curRow_Buffer[c].Char.UnicodeChar = L' ';
				curRow_Buffer[c].Attributes = 0b0111;
			}
			curRow_Buffer[w-1].Char.UnicodeChar = charset[1];
			curRow_Buffer[w-1].Attributes = 0b0111;
			WriteConsoleOutputW(
					hConsole,
					curRow_Buffer,
					row_Size,
					upper_left_Coord,
					&curRow_Region);
			free(curRow_Buffer);
		} else {
			SMALL_RECT curRow_left_Region = { x,r+y, x,r+y };
			CHAR_INFO curRow_left_Buffer[1] = {};
			curRow_left_Buffer[0].Char.UnicodeChar = charset[3];
			curRow_left_Buffer[0].Attributes = 0b0111;
			WriteConsoleOutputW(
					hConsole,
					curRow_left_Buffer,
					side_Size,
					upper_left_Coord,
					&curRow_left_Region);

			SMALL_RECT curRow_right_Region = { w-1+x,r+y, w-1+x,r+y };
			CHAR_INFO curRow_right_Buffer[1] = {};
			curRow_right_Buffer[0].Char.UnicodeChar = charset[1];
			curRow_right_Buffer[0].Attributes = 0b0111;
			WriteConsoleOutputW(
					hConsole,
					curRow_right_Buffer,
					side_Size,
					upper_left_Coord,
					&curRow_right_Region);
		}
	}

	// bottom bit
	SMALL_RECT bottom_Region = { x,h-1+y, w+x,h-1+y };
	CHAR_INFO *bottom_Buffer = (CHAR_INFO*) calloc(w, sizeof(CHAR_INFO));
	bottom_Buffer[0].Char.UnicodeChar = charset[7];
	bottom_Buffer[0].Attributes = 0b0111;
	for (uhex c=1; c <= (w-2); ++c) {
		bottom_Buffer[c].Char.UnicodeChar = charset[2];
		bottom_Buffer[c].Attributes = 0b0111;
	}
	bottom_Buffer[w-1].Char.UnicodeChar = charset[6];
	bottom_Buffer[w-1].Attributes = 0b0111;
	WriteConsoleOutputW(
			hConsole,
			bottom_Buffer,
			row_Size,
			upper_left_Coord,
			&bottom_Region);
	free(bottom_Buffer);

	return;
}
/*
void printbox_color(
	uhex w,
	uhex h,
	uhex x,
	uhex y,
	const wchar_t charset[8],
	ubyte fgclr,
	ubyte bgclr,
	int solid
) {
	printf("sorry bro this function (`printbox_color`) isn't supported on windows just yet.");

	return;
}
*/
#else
void printbox(
	uhex w, 
	uhex h, 
	uhex x, 
	uhex y, 
	const wchar_t charset[8], 
	int solid
) {
	wprintf(L"\x1B[%lu;%luf%lc", y+1, x+1, charset[4]);
	for (uhex c=0; c<(w-2); ++c) {
		wprintf(L"%lc", charset[0]);
	}
	wprintf(L"%lc\x1B[1B\x1B[%luD", charset[5], w);
	
	for (uhex r=0; r<(h-2); ++r) {
		wprintf(L"%lc", charset[3]);
		if (solid) {
			for (uhex c=0; c<(w-2); ++c) {
				wprintf(L" ");
			}
		} else {
			wprintf(L"\x1B[%luC", w-2);
		}
		wprintf(L"%lc\x1B[1B\x1B[%luD", 
			charset[1], w);
	}

	wprintf(L"%lc", charset[7]);
	for (uhex c=0; c<(w-2); ++c) {
		wprintf(L"%lc", charset[2]);
	}
	wprintf(L"%lc", charset[6]);
}
void printbox_color(uhex w, uhex h, uhex x, uhex y, const wchar_t charset[8], ubyte fgclr, ubyte bgclr, int solid) {
	wprintf(L"\x1B[38;5;%dm\x1B[48;5;%dm", fgclr, bgclr);
	printbox(w, h, x, y, charset, solid);
	wprintf(L"\x1B[0m");
}
#endif

// very much WIP!!
// wait actually it works now but it's probably pretty slow
void printgrid(
	uhex grid_w, 
	uhex grid_h, 
	uhex grid_nx, 
	uhex grid_ny,
	uhex x,
	uhex y,
	const wchar_t charset[13]) 
{
	if (grid_nx == 0 || grid_ny == 0) {
		return;
	}
	const uhex TOTAL_WIDTH = grid_nx*(grid_w-1)+1;

	// top row
	wprintf(L"\x1B[%lu;%luf%lc", y+1, x+1, charset[4]);
	for (uhex c=0; c<(grid_nx-1); ++c) {
		for (uhex i=0; i<(grid_w-2); ++i) {
			wprintf(L"%lc", charset[0]);
		}
		wprintf(L"%lc", charset[8]);
	}
	for (uhex i=0; i<(grid_w-2); ++i) {
		wprintf(L"%lc", charset[0]);
	}
	wprintf(L"%lc", charset[5]);

	// between rows
	for (uhex r=0; r<(grid_ny-1); ++r) {
		for (uhex i=0; i<(grid_h-2); ++i) {
			wprintf(L"\x1B[1B\x1B[%luD%lc", 
				TOTAL_WIDTH,
				charset[3]);
			for (uhex c=0; c<grid_nx; ++c) {
				wprintf(L"\x1B[%luC%lc", grid_w-2, charset[3]);
			}
		}
		wprintf(L"\x1B[1B\x1B[%luD%lc",
			TOTAL_WIDTH,
			charset[11]);
		for (uhex c=0; c<(grid_nx-1); ++c) {
			for (uhex i=0; i<(grid_w-2); ++i) {
				wprintf(L"%lc", charset[0]);
			}
			wprintf(L"%lc", charset[12]);
		}
		for (uhex i=0; i<(grid_w-2); ++i) {
			wprintf(L"%lc", charset[0]);
		}
		wprintf(L"%lc", charset[9]);
	}

	for (uhex i=0; i<(grid_h-2); ++i) {
		wprintf(L"\x1B[1B\x1B[%luD%lc",
			TOTAL_WIDTH,
			charset[3]);
		for (uhex c=0; c<grid_nx; ++c) {
			wprintf(L"\x1B[%luC%lc", grid_w-2, charset[3]);
		}
	}
	
	// bottom row
	wprintf(L"\x1B[1B\x1B[%luD%lc", 
		TOTAL_WIDTH, 
		charset[7]);
	for (uhex c=0; c<(grid_nx-1); ++c) {
		for (uhex i=0; i<(grid_w-2); ++i) {
			wprintf(L"%lc", charset[0]);
		}
		wprintf(L"%lc", charset[10]);
	}
	for (uhex i=0; i<(grid_w-2); ++i) {
		wprintf(L"%lc", charset[0]);
	}
	wprintf(L"%lc", charset[6]);
}
