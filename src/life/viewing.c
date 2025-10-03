#include <time.h>
#include <stdbool.h>
#include <inttypes.h>
#include <wchar.h>
#include <stdio.h>

#include "border.h"
#include "runtime_flags.h"
#include "big_header.h"
#include "viewing.h"

const wchar_t alive_cell_full   = L'\x2588';
const wchar_t alive_cell_top    = L'\x2580';
const wchar_t alive_cell_bottom = L'\x2584';
const wchar_t dead_cell_full    = L' ';

extern uint16_t ROWS, COLS;
extern uint8_t runtime_flags;

#ifdef _WIN32
#include <windows.h>

#define _PAUSED_LEN 6

void display_paused_notif(void)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD note_Size = { _PAUSED_LEN, 1 };
	COORD upper_left_Coord = { 0, 0 };
	SMALL_RECT note_Region = { (COLS/2)-2,(ROWS/2)+1, (COLS/2)+4,(ROWS/2)+1 };
	CHAR_INFO note_Buffer[6] = {0};
	const wchar_t note_display[_PAUSED_LEN+1] = L"PAUSED";
	for (int8_t i = 0; i < _PAUSED_LEN; ++i) {
		note_Buffer[i].Char.UnicodeChar = note_display[i];
		note_Buffer[i].Attributes = 0x7;
	}
	WriteConsoleOutputW(
			hConsole,
			note_Buffer,
			note_Size,
			upper_left_Coord,
			&note_Region);
}

void print_grid(uint8_t *grid)
{
	clock_t start;
	IFDBG start = clock();
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "FAILED TO OBTAIN CONSOLE HANDLE (windows.h)");
		return;
	}

	COORD row_Size = { COLS,1 };
	COORD upper_left_Coord = { 0, 0 };

	CHAR_INFO *curRow_Buffer = calloc(COLS, sizeof(*curRow_Buffer));
	for (uint8_t row = 0; row < ROWS; row += 2) {
		SMALL_RECT curRow_Region = { 1,1+(row/2), COLS,1+(row/2) };
		for (uint16_t col = 0; col < COLS; ++col) {
			if (row == 0) {
				curRow_Buffer[col].Attributes = 0x7;
			}

			uint8_t top_cell = GETBIT(grid[((COLS/8)*row + (col/8))], col%8);
			if (row == (ROWS-1)) {
				curRow_Buffer[col].Char.UnicodeChar = (top_cell) ? alive_cell_top : dead_cell_full;
				continue;
			}
			uint8_t bottom_cell = GETBIT(grid[((COLS/8)*(row+1) + (col/8))], col%8);
			
			if (top_cell && bottom_cell)
				curRow_Buffer[col].Char.UnicodeChar = alive_cell_full;
			else if (top_cell != bottom_cell)
				curRow_Buffer[col].Char.UnicodeChar = (top_cell) ? alive_cell_top : alive_cell_bottom;
			else
				curRow_Buffer[col].Char.UnicodeChar = dead_cell_full;
		}
		WriteConsoleOutputW(
				hConsole,
				curRow_Buffer,
				row_Size,
				upper_left_Coord,
				&curRow_Region);
	}

	IFDBG {
		clock_t end = clock();
		double elapsed = (double) (end - start) / CLOCKS_PER_SEC;
		wprintf(L"\x1b[1;1f%.7lf seconds", elapsed);
	}
}

void help_screen(void)
{
	uint16_t row_change = 0;
	if (COLS < HELP_MENU_WIDTH
	|| (ROWS/2) < HELP_MENU_HEIGHT)
		row_change = (ROWS/4)+6;

	printbox(
		HELP_MENU_WIDTH, HELP_MENU_HEIGHT,
		(row_change == 0) ? COLS/2 - (HELP_MENU_WIDTH-2)/2 : 0,
		(ROWS/4)-2 + row_change,
		W1T1_BORDERCHARS,
		true);

	uint16_t write_col =
		(row_change == 0)
			? ((COLS/2)-(HELP_MENU_WIDTH-2)/2 + 1+10-9)
			: 2;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD row_Size = { 28, 1 };
	COORD upper_left_Coord = { 0, 0 };
	SMALL_RECT msg_Regions[6] = {0};
	msg_Regions[0] = (SMALL_RECT){
		(row_change == 0) ? (COLS/2)-8 : 6,
		(ROWS/4)-1 + row_change-1,
		(row_change == 0) ? (COLS/2)-8+17 : 23,
		(ROWS/4)-1 + row_change-1
	};
	
	for (int8_t i = 1; i < 6; ++i)
		msg_Regions[i] = (SMALL_RECT){
			write_col-1, (ROWS/4) + row_change + i-2,
			write_col+(HELP_MENU_WIDTH-2) - 1, (ROWS/4) + row_change + i-2
		};
	CHAR_INFO msg_Buffers[28][6] = {0};
	const wchar_t *msgs[6] = {
		L"KEYBOARD SHORTCUTS",
		L"q: quit the program         ",
		L"p: (un)pause the simulation ",
		L"n: advance by one generation",
		L"?: show this help message   ",
		L"esc: exit menus (like this) ",
	};
	for (size_t c = 0; c < 28; ++c) {
		msg_Buffers[0][c].Char.UnicodeChar = msgs[0][c];
		msg_Buffers[0][c].Attributes = 0x7;
	}
	WriteConsoleOutputW(
			hConsole, 
			msg_Buffers[0], 
			row_Size, 
			upper_left_Coord, 
			&msg_Regions[0]);
	for (int8_t i = 1; i < 6; ++i) {
		for (size_t c = 0; c < 28; ++c) {
			msg_Buffers[i][c].Char.UnicodeChar = msgs[i][c];
			msg_Buffers[i][c].Attributes = 0x7;
		}
		WriteConsoleOutputW(
				hConsole, 
				msg_Buffers[i], 
				row_Size, 
				upper_left_Coord, 
				&msg_Regions[i]);
	}
}

void close_help_screen(uint8_t *grid)
{
	if (COLS >= HELP_MENU_WIDTH && (ROWS/2) >= HELP_MENU_HEIGHT)
		print_grid(grid);
	else
		printbox(
			HELP_MENU_WIDTH, HELP_MENU_HEIGHT,
			0, (ROWS/2)+3,
			INVIS_BORDERCHARS,
			true);
}
#else
void display_paused_notif(void)
{
	wprintf(L"\x1b[%" PRIu16 L";%" PRIu16 L"fPAUSED", ROWS/2 + 2, COLS/2 - 1);
	fflush(stdout);
}

void print_grid(uint8_t *grid)
{
	clock_t start;
	IFDBG start = clock();

	wprintf(L"\x1b[2;2f");

	for (uint16_t row = 0; row < ROWS; row += 2) {
		for (uint16_t col = 0; col < COLS; ++col) {
			uint8_t top_cell = GETBIT(grid[((COLS/8)*row + (col/8))], col%8);
			if (row == (ROWS-1)) {
				wprintf(L"%lc", (top_cell == 0) ? dead_cell_full : alive_cell_top);
				continue;
			}

			uint8_t bottom_cell = GETBIT(grid[((COLS/8)*(row+1) + (col/8))], col%8);
			if (top_cell && bottom_cell)
				wprintf(L"%lc", alive_cell_full);
			else if (top_cell != bottom_cell)
				wprintf(L"%lc", (top_cell) ? alive_cell_top : alive_cell_bottom);
			else
				wprintf(L"%lc", dead_cell_full);
		}
		wprintf(L"\x1b[1B\x1b[%" PRIu16 L"D", COLS);
	}
	wprintf(L"\n\x1b[%" PRIu16 L";1f", (ROWS/2)+3);

	fflush(stdout);

	IFDBG {
		clock_t end = clock();
		double elapsed = (double) (end - start) / CLOCKS_PER_SEC;
		wprintf(L"\x1b[1;1f%.7lf seconds", elapsed);
	}
}

void help_screen(void)
{
	uint16_t row_change = 0;
	if (COLS < HELP_MENU_WIDTH
	|| (ROWS/2) < HELP_MENU_HEIGHT)
		row_change = (ROWS/4) + 6;

	printbox(
		HELP_MENU_WIDTH, HELP_MENU_HEIGHT, 
		(row_change == 0) ? (COLS/2) - (HELP_MENU_WIDTH-2)/2 : 0,
		(ROWS/4)-2 + row_change,
		W1T1_BORDERCHARS,
		true);

	uint16_t write_col = 
		(row_change == 0)
			? (COLS/2) - (HELP_MENU_WIDTH-2)/2 + 1+10-9
			: 2;

	wprintf(L"\x1b[%lu;%luf"	"KEYBOARD_SHORTCUTS",
			(ROWS/4)-1 + row_change,
			(row_change == 0)
				? (COLS/2) - (HELP_MENU_WIDTH-2)/2 + 6+10-9
				: 7);
	wprintf(L"\x1b[%lu;%luf" "q: quit the program"
		  L"\x1b[%lu;%luf" "p: (un)pause the simulation"
		  L"\x1b[%lu;%luf" "n: advance by one generation"
		  L"\x1b[%lu;%luf" "?: show this help message"
		  L"\x1b[%lu;%luf" "esc: exit menus (like this)",
		  (ROWS/4)+row_change,   write_col,
		  (ROWS/4)+row_change+1, write_col,
		  (ROWS/4)+row_change+2, write_col,
		  (ROWS/4)+row_change+3, write_col,
		  (ROWS/4)+row_change+4, write_col);

	fflush(stdout);
}

void close_help_screen(uint8_t *grid)
{
	if (COLS >= HELP_MENU_WIDTH && (ROWS/2) >= HELP_MENU_HEIGHT)
		print_grid(grid);
	else {
		printbox(
			HELP_MENU_WIDTH, HELP_MENU_HEIGHT, 
			0, (ROWS/2)+3,
			INVIS_BORDERCHARS,
			true);
		fflush(stdout);
	}
}
#endif
