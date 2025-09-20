#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#ifdef _WIN32
	#include <windows.h>
	#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
#endif


#include "border.h"
#include "viewing.h"
#include "big_header.h"
#include "arg_parse.h"

/* COLS//8 = number of `cell8`s in each row
 * BIT(grid[((COLS>>3)*row + (col>>3))], col%8) = the value of cell with col in range [0,COLS), row in range [0,ROWS)
 * cell//COLS = row of cell with cell in range [0, COLS*ROWS)
 * cell%COLS = col of cell with cell in range [0, COLS*ROWS)
 */

const char *PROG_NAME;
uint16_t ROWS = 48;
uint16_t COLS = 48;
uint16_t CC = 6; // COLS>>3;
uint16_t N_CELLS = 2304; // ROWS*COLS;
uint16_t GRID_EL_N = 288; // N_CELLS>>3;

uint8_t *grid;
uint8_t *prev_grid;
bool bin_loaded = false;

uint8_t runtime_flags = 0;


void sleepns(uint64_t ns)
{
	struct timespec tim, tim2;
	tim.tv_sec = (time_t) (ns/(1000000000)); // whole number of seconds
	tim.tv_nsec = (int32_t) (ns%(1000000000));

	nanosleep(&tim, &tim2);
}

void set_dims(uint16_t rows, uint16_t cols)
{
	ROWS = rows;
	COLS = cols;
	CC = COLS/8;
	N_CELLS = ROWS*COLS;
	GRID_EL_N = N_CELLS/8;
}

//void term_too_small_display(uhex rows, uhex cols)
//{
//	wprintf(L"\x1b[2J\x1b[;f%lux%lu", cols, rows);
//}

/* Rules:
 * <2: dies
 * 2 or 3: lives
 * >3: dies
 * dead cell with 3: becomes alive
 */
void update(void)
{
	for (uint16_t i=0; i < (N_CELLS); ++i) {
		uint8_t num_neighbors = 0;
		// cur cell: BIT(prev_grid[i>>3], i%8)
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

				num_neighbors += GETBIT(prev_grid[neighbor_i/8], neighbor_i%8);
			}
		}

		if ((GETBIT(prev_grid[i/8], i%8) && num_neighbors >= 2 && num_neighbors <= 3) || 
		   (!GETBIT(prev_grid[i/8], i%8) && num_neighbors == 3))
			SETBIT(grid[i/8], i%8);
	}
}

int32_t main(int32_t argc, char **argv)
{
	// define the locale to support Windows and other platforms (since Windows wants a different value for some reason)
	cp_set_unicode_locale();
	fwide(stdout, 1);
	
	arg_parse(argc, argv);
	
	prev_grid = calloc(GRID_EL_N, sizeof(uint8_t));

	//printf("finished checking command line arguments\n");
	if (!bin_loaded) {
		grid = (uint8_t *) calloc(GRID_EL_N, sizeof(uint8_t));
		if (grid == NULL) {
			fprintf(stderr, "failed to allocate memory for grid");
			exit(1);
		}
	
		// initialize the default cell positions (this can be changed, but you have to use `cst`)
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
	}
	
	wprintf(L"\x1b[2J\x1b[?25l"); // hides the cursor and clears the screen
	print_grid(grid); // clear the screen
	
	printbox(
		COLS+2,(ROWS/2)+2,
		0,0,
		W1T1_BORDERCHARS,
		false);

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
		if (kbhit() && (ch = getch())) {
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
						memcpy(prev_grid, grid, GRID_EL_N);
						memset(grid, 0, GRID_EL_N);
						update();
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
						close_help_screen(grid);
					}
					break;
				case 0x1b: // the escape key
					in_a_menu = 0;
					close_help_screen(grid);
					break;
			}
		}

		if (ready_to_quit) {
			if (in_a_menu) {
				in_a_menu = 0;
				close_help_screen(grid);
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
			memcpy(prev_grid, grid, GRID_EL_N);
			memset(grid, 0, GRID_EL_N);
			update();
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
	free(prev_grid);

	return 0;
}
