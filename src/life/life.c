#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif


#include "border.h"
#include "border_chars.h"

#define BIT(num, i) ((num>>(7-i))&1)
#define SETBIT(num, i) num |= 1<<(7-i);

#define HELP_MENU_WIDTH 30
#define HELP_MENU_HEIGHT 7

uint16_t ROWS = 48;
uint16_t COLS = 48;
uint16_t CC = 6; // COLS>>3;
uint16_t N_CELLS = 2304; // ROWS*COLS;
uint16_t GRID_EL_N = 288; // N_CELLS>>3;

/* COLS//8 = number of `cell8`s in each row
 * BIT(grid[((COLS>>3)*row + (col>>3))], col%8) = the value of cell with col in range [0,COLS), row in range [0,ROWS)
 * cell//COLS = row of cell with cell in range [0, COLS*ROWS)
 * cell%COLS = col of cell with cell in range [0, COLS*ROWS)
 */

const wchar_t alive_cell_full = L'\x2588';
const wchar_t alive_cell_top = L'\x2580';
const wchar_t alive_cell_bottom = L'\x2584';
const wchar_t dead_cell_full = L' ';

void sleepns(unsigned long long ns)
{
	struct timespec tim, tim2;
	tim.tv_sec = (time_t) (ns/(1000000000)); // whole number of seconds
	tim.tv_nsec = (int32_t) (ns%(1000000000));

	nanosleep(&tim, &tim2);
}

//void term_too_small_display(uhex rows, uhex cols)
//{
//	wprintf(L"\x1b[2J\x1b[;f%lux%lu", cols, rows);
//}

void display_paused_notif()
{
#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	// all the '6's here are just because that's the length of the string 'PAUSED'
	COORD note_Size = { 6, 1 };
	COORD zerozero = { 0, 0 };
	SMALL_RECT note_Region = { (COLS/2)-2,(ROWS/2)+1, (COLS/2)+4,(ROWS/2)+1 };
	CHAR_INFO note_Buffer[6] = {0};
	const wchar_t note_display[6] = L"PAUSED";
	for (int8_t i=0; i<6; ++i) {
		note_Buffer[i].Char.UnicodeChar = note_display[i];
		note_Buffer[i].Attributes = 0x7;
	}
	WriteConsoleOutputW(
			hConsole,
			note_Buffer,
			note_Size,
			zerozero,
			&note_Region);
#else
	wprintf(L"\x1b[%" PRIu16 L";%" PRIu16 L"fPAUSED", ROWS/2 + 2, COLS/2 - 1);
	fflush(stdout);
#endif
}

#ifdef _WIN32
// windows version of the print_grid function
void print_grid(uint8_t *grid)
{
	#ifdef LIFE_DEBUG
	clock_t start = clock();
	#endif
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		printf("FAILED TO OBTAIN CONSOLE HANDLE (windows.h)");
		return;
	}

	COORD row_Size = { COLS,1 };
	COORD upper_left_Coord = { 0,0 };

	CHAR_INFO *curRow_Buffer = (CHAR_INFO*) calloc(COLS, sizeof(CHAR_INFO));
	for (uint16_t row=0; row<ROWS; row+=2) {
		SMALL_RECT curRow_Region = { 1,1+(row/2), COLS,1+(row/2) };
		for (uint16_t col=0; col<COLS; ++col) {
			if (row == 0) {
				curRow_Buffer[col].Attributes = 0x7; // white (combines red, green, and blue parts)
			}

			uint8_t top_cell = BIT(grid[((COLS/8)*row + (col/8))], col%8);
			if (row == (ROWS-1)) {
				curRow_Buffer[col].Char.UnicodeChar = ((top_cell) ? alive_cell_top : dead_cell_full);
				continue;
			}
			uint8_t bottom_cell = BIT(grid[((COLS/8)*(row+1) + (col/8))], col%8);

			if (top_cell && bottom_cell) {
				curRow_Buffer[col].Char.UnicodeChar = alive_cell_full;
			} else if (top_cell != bottom_cell) {
				curRow_Buffer[col].Char.UnicodeChar = ((top_cell) ? alive_cell_top : alive_cell_bottom);
			} else {
				curRow_Buffer[col].Char.UnicodeChar = dead_cell_full;
			}
		}
		WriteConsoleOutputW(
				hConsole,
				curRow_Buffer,
				row_Size,
				upper_left_Coord,
				&curRow_Region);
	}

	#ifdef LIFE_DEBUG
	clock_t end = clock();
	double elapsed = (double) (end-start) / CLOCKS_PER_SEC;
	wprintf(L"\x1b[1;1f%.7lf seconds", elapsed);
	#endif
}
#else
// not windows version of the print_grid function
void print_grid(uint8_t *grid)
{
	#ifdef LIFE_DEBUG
	clock_t start = clock();
	#endif
	wprintf(L"\x1b[2;2f");
	// we handle the rows in pairs because it makes the cells look more square
	for (uint16_t row = 0; row < ROWS; row += 2) {
		for (uint16_t col = 0; col < COLS; ++col) {
			uint8_t top_cell = BIT(grid[((COLS/8)*row + (col/8))], col%8);
			if (row == (ROWS-1)) {
				wprintf(L"%lc", ((top_cell==0) ? dead_cell_full : alive_cell_top));
				continue;
			}
			uint8_t bottom_cell = BIT(grid[((COLS/8)*(row+1) + (col/8))], col%8);
			if (top_cell && bottom_cell) {
				wprintf(L"%lc", alive_cell_full);
			} else if (top_cell != bottom_cell) {
				wprintf(L"%lc", ((top_cell) ? alive_cell_top : alive_cell_bottom));
			} else {
				wprintf(L"%lc", dead_cell_full);
			}
		}
		wprintf(L"\x1b[1B\x1b[%" PRIu16 L"D", COLS);
	}
	wprintf(L"\n\x1b[%" PRIu16 L";1f", (ROWS/2)+3);

	fflush(stdout);

	#ifdef LIFE_DEBUG
	clock_t end = clock();
	double elapsed = (double) (end-start)/CLOCKS_PER_SEC;
	wprintf(L"\x1b[1;1f%.7lf seconds", elapsed);
	#endif
}
#endif

/* Rules:
 * <2: dies
 * 2 or 3: lives
 * >3: dies
 * dead cell with 3: becomes alive
 */
uint8_t *update(uint8_t *grid)
{
	uint8_t *new_grid = (uint8_t *) calloc(GRID_EL_N, sizeof(uint8_t));

	for (uint16_t i=0; i < (N_CELLS); ++i) {
		uint8_t num_neighbors = 0;
		// cur cell: BIT(grid[i>>3], i%8)
		uint16_t cell_col = i%COLS;
		uint16_t cell_row = i/COLS;

		for (int8_t dy = -1; dy <= 1; ++dy) {
			for (int8_t dx = -1; dx <= 1; ++dx) {
				if (dx==0 && dy == 0) continue;
				
				// we'll treat any neighboring "cell" outside the grid boundaries as dead
				if ((cell_col == 0      && dx == -1) 
				 || (cell_col == COLS-1 && dx ==  1)
				 || (cell_row == 0      && dy == -1)
				 || (cell_row == ROWS-1 && dy ==  1)) continue;

				uint16_t neighbor_i = i + dx + (dy*COLS);

				num_neighbors += BIT(grid[neighbor_i/8], neighbor_i%8);
			}
		}

		if ((BIT(grid[i/8], i%8) && num_neighbors >= 2 && num_neighbors <= 3) || 
		   (!BIT(grid[i/8], i%8) && num_neighbors == 3))
			SETBIT(new_grid[i/8], i%8);
	}

	return new_grid;
}

void help_screen(void)
{
	uint16_t row_change = 0;
	if (COLS < HELP_MENU_WIDTH 
	|| (ROWS/2) < HELP_MENU_HEIGHT)
		row_change = (ROWS/4)+6;

	printbox(
		HELP_MENU_WIDTH, HELP_MENU_HEIGHT, //size
		(row_change==0) ? ((COLS/2)-(HELP_MENU_WIDTH-2)/2) : (0),
		(ROWS/4)-2+row_change,
		W1T1_BORDERCHARS,
		1);

	uint16_t write_col = 
		(row_change==0) 
			? ((COLS/2)-(HELP_MENU_WIDTH-2)/2+1+10-9) 
			: 2;
	#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD row_Size = { 28, 1 };
	COORD zerozero = { 0, 0 };
	SMALL_RECT msg_Regions[6] = {0};
	msg_Regions[0] = (SMALL_RECT){
		(row_change==0) ? ((COLS/2)-8) : 6,
		(ROWS/4)-1+row_change-1,
		(row_change==0) ? ((COLS/2)-8+17) : 23,
		(ROWS/4)-1+row_change-1
	};
	for (int8_t i=1; i<6; ++i)
		msg_Regions[i] = (SMALL_RECT){
			write_col-1, (ROWS/4)+row_change+i-2,
			write_col+(HELP_MENU_WIDTH-2)-1, (ROWS/4)+row_change+i-2
		};
	CHAR_INFO msg_Buffers[28][6] = {0};
	const wchar_t* msgs[6] = {
		L"KEYBOARD SHORTCUTS",
		L"q: quit the program         ",
		L"p: (un)pause the simulation ",
		L"n: advance by one generation",
		L"?: show this help message   ",
		L"esc: exit menus (like this) ",
	};
	for (size_t c=0; c<28; ++c) {
		msg_Buffers[0][c].Char.UnicodeChar = msgs[0][c];
		msg_Buffers[0][c].Attributes = 0x7;
	}
	WriteConsoleOutputW(
			hConsole,
			msg_Buffers[0],
			row_Size,
			zerozero,
			&msg_Regions[0]);
	for (int8_t i=1; i<6; ++i) {
		for (size_t c=0; c<28; ++c) {
			msg_Buffers[i][c].Char.UnicodeChar = msgs[i][c];
			msg_Buffers[i][c].Attributes = 0x7;
		}
		WriteConsoleOutputW(
				hConsole,
				msg_Buffers[i],
				row_Size,
				zerozero,
				&msg_Regions[i]);
	}
	#else
	wprintf(L"\x1b[%lu;%lufKEYBOARD SHORTCUTS", 
			(ROWS/4)-1+row_change,
			(row_change==0)
				? ((COLS/2)-(HELP_MENU_WIDTH-2)/2+6+10-9) 
				: 7);
	wprintf(L"\x1b[%lu;%lufq: quit the program"
			L"\x1b[%lu;%lufp: (un)pause the simulation"
			L"\x1b[%lu;%lufn: advance by one generation"
			L"\x1b[%lu;%luf?: show this help message"
			L"\x1b[%lu;%lufesc: exit menus (like this)",
			(ROWS/4)+row_change,   write_col,
			(ROWS/4)+row_change+1, write_col,
			(ROWS/4)+row_change+2, write_col,
			(ROWS/4)+row_change+3, write_col,
			(ROWS/4)+row_change+4, write_col);
	// there's gotta be a better way to write that, right?

	fflush(stdout);
	#endif
}

int main(int argc, char *argv[])
{
	// define the locale to support Windows and other platforms (since Windows wants a different value for some reason)
	cp_set_unicode_locale();
	fwide(stdout, 1);
	

	if (argc > 1) { // arguments should be in "COLSxROWS"
		for (size_t i=0; i < strlen(argv[1]); ++i) {
			if (argv[1][i] == 'x') {
				char* tmp2 = (char*) malloc(sizeof(char) * (i+1));
				char* tmp3 = (char*) malloc(sizeof(char) * (strlen(argv[1])-1));
				strcpy(tmp2, argv[1]);
				for (size_t j=(i+1); j<strlen(argv[1]); ++j) {
					tmp3[j-i-1] = argv[1][j];
				}

				/* the expression below rounds it up to the nearest multiple of 8 (or 2 for rows) because 
				 * i can't be bothered to fix my code to work with dimensions that aren't multiples of 8
				 */
				ROWS = (atoi(tmp3)+1) & ~1; 
				COLS = (atoi(tmp2)+7) & ~7;
				CC = COLS/8;
				N_CELLS = ROWS*COLS;
				GRID_EL_N = N_CELLS/8;
			}
		}
	}
	
	//printf("finished checking command line arguments\n");

	uint8_t *grid = (uint8_t *) calloc(GRID_EL_N, sizeof(uint8_t));
	
	// initialize the default cell positions (this can be changed, but i haven't made an easier way to do that just yet)
	grid[ 4*CC+4] = 0x20;
	grid[ 5*CC+2] = 0x08;
	grid[ 5*CC+4] = 0x80;
	grid[ 9*CC+2] = 0xA0;
	grid[ 9*CC+3] = 0x04;
	grid[10*CC+3] = 0x0C;
	grid[11*CC+2] = 0x02;
	grid[11*CC+3] = 0x20;
	grid[11*CC+4] = 0x40;
	grid[12*CC+2] = 0x10;
	grid[12*CC+3] = 0x12;
	grid[13*CC+2] = 0x10;
	grid[13*CC+3] = 0xC0;
	grid[13*CC+4] = 0xC0;
	grid[14*CC+2] = 0x13;
	grid[14*CC+4] = 0x20;
	grid[15*CC+2] = 0x04;
	grid[15*CC+3] = 0x20;
	grid[16*CC+3] = 0x08;
	grid[16*CC+4] = 0x04;
	grid[17*CC+4] = 0x40;
	grid[18*CC+2] = 0x07;
	grid[19*CC+3] = 0x02;
	grid[20*CC+3] = 0x18;
	grid[22*CC+2] = 0x04;
	grid[22*CC+3] = 0x80;
	
	wprintf(L"\x1b[2J\x1b[?25l"); // clears the screen and hides the cursor
	print_grid(grid);
	
	printbox(
		COLS+2,(ROWS/2)+2,
		0,0,
		W1T1_BORDERCHARS,
		true);

	//int was_too_small = 0;
	
	char ch;
	int ready_to_quit = 0;
	int in_a_menu = 0;
	int paused = 0;
	
	#ifndef _WIN32
	struct termios old_term, new_term;
	tcgetattr(STDIN_FILENO, &old_term);
	new_term = old_term;
	new_term.c_lflag &= ~(ICANON | ECHO);
	new_term.c_cc[VMIN] = 0;
	new_term.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
	#endif

	while (1) {
		// keyboard input polling (moved into this loop from a separate thread because apparently a thread sucks for this)
		#ifdef _WIN32
		if (kbhit()) {
			ch = getch();
		#else
		if (read(STDIN_FILENO, &ch, 1) == 1) {
		#endif
			switch (ch) {
				case 'q':
				case 'Q':
					ready_to_quit = 1;
					break;
				case 'p':
				case 'P':
					if (!in_a_menu) {
						paused ^= 1; // efficiently invert the value of `paused`
						if (paused)
							display_paused_notif();
						else
							printbox(
								COLS+2,(ROWS/2)+2,
								0,0,
								W1T1_BORDERCHARS,
								false);
					}
					break;
				case 'n':
				case 'N':
					if (paused && !in_a_menu) {
						// it'd be weird if this did something even when it isn't paused
						uint8_t *new_grid = update(grid);
						memcpy(grid, new_grid, GRID_EL_N);
						free(new_grid);
						print_grid(grid);
					}
					break;
				case 'h':
				case 'H':
				case '?':
					in_a_menu = !in_a_menu;
					if (in_a_menu) {
						paused = 1;
						display_paused_notif();
						help_screen();
					} else {
						if (COLS<HELP_MENU_WIDTH || (ROWS/2)<HELP_MENU_HEIGHT) {
							// if COLS and ROWS are within certain parameters, 
							printbox(
								HELP_MENU_WIDTH,HELP_MENU_HEIGHT,
								0,(ROWS/2)+3,
								INVIS_BORDERCHARS,
								true);
						#ifndef _WIN32
							fflush(stdout);
						#endif
						} else
							print_grid(grid);
					}
					break;
				case 0x1b: // the escape key
					in_a_menu = 0;
					if (COLS<HELP_MENU_WIDTH || (ROWS/2)<HELP_MENU_HEIGHT) {
						printbox(
							HELP_MENU_WIDTH,HELP_MENU_HEIGHT,
							0,(ROWS/2)+3,
							INVIS_BORDERCHARS,
							true);
					#ifndef _WIN32
						fflush(stdout);
					#endif
					} else {
						print_grid(grid);
					}
					break;
			}
		}

		if (ready_to_quit) {
			if (in_a_menu) {
				in_a_menu = 0;
				if (COLS>=30)
					print_grid(grid);
				else {
					printbox(
						30,7,
						0,(ROWS/2)+3,
						INVIS_BORDERCHARS,
						true);
					fflush(stdout);
				}
			}
			wprintf(L"\x1b[%" PRIu16 L";1f", (ROWS/2)+3);
			break;
		}
		// check if terminal size is too small
		/*uint16_t rows, cols;
		get_term_size(&rows, &cols);
		if ((rows<((ROWS>>1)+1)) || (cols<(COLS+2))) {
			term_too_small_display(rows, cols);
			was_too_small = 1;
		} else {
			if (was_too_small) {
				was_too_small = 0;
				printbox(
					COLS+2,(ROWS>>1)+2,
					0,0,
					W1T1_BORDERCHARS,
					true);
			}
			print_grid(grid);
		}*/

		// generate the new grid and update the old one
		if (!paused) {
			uint8_t *new_grid = update(grid);
			memcpy(grid, new_grid, GRID_EL_N);
			free(new_grid);
			print_grid(grid);
		}

		#ifdef _WIN32
		sleepns(1000000000/20 - 10000000);
		#else
		sleepns(1000000000/20); // sleeps to keep a framerate (`sleepns` takes nanoseconds)
		#endif
	}

	#ifndef _WIN32
	tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
	wprintf(L"\x1b[?25h");
	#endif

	free(grid);

	return 0;
}
