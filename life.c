#include <wchar.h>
#include <locale.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
  #include <conio.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif


#include "border.c"

volatile int ready_to_quit;
volatile int paused;
volatile int just_unpaused;
volatile int just_paused;
volatile int next_gen;

#define BIT(num, i, size) ((num>>(size-1-i))&1)
#define SETBIT(num, i, size) num |= 1<<(size-1-i);

uhex ROWS = 48;
uhex COLS = 48;
uhex CC = 6; // COLS>>3;
uhex N_CELLS = 2304; // ROWS*COLS;
uhex GRID_EL_N = 288; // N_CELLS>>3;

/* COLS//8 = number of `cell8`s in each row
 * BIT(grid[((COLS>>3)*row + (col>>3))], col%8, 8) = the value of cell with col in range [0,COLS), row in range [0,ROWS)
 * cell//COLS = row of cell with cell in range [0, COLS*ROWS)
 * cell%COLS = col of cell with cell in range [0, COLS*ROWS)
 */

const wchar_t alive_cell_full = L'\x2588';
const wchar_t alive_cell_top = L'\x2580';
const wchar_t alive_cell_bottom = L'\x2584';
const wchar_t dead_cell_full = /*L'\x2591'*/L' ';

typedef uint_fast8_t cell8;

void sleepns(unsigned long long ns) {
	struct timespec tim, tim2;
	tim.tv_sec = (time_t) (ns/(1000000000)); // whole number of seconds
	tim.tv_nsec = (long) (ns%(1000000000));

	nanosleep(&tim, &tim2);
}

void term_too_small_display(uhex rows, uhex cols) {
	wprintf(L"\x1b[2J\x1b[;f%lux%lu", cols, rows);
}

#ifdef _WIN32
// windows version of the print_grid function
void print_grid(cell8* grid) {
	//clock_t start = clock();
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		printf("FAILED TO OBTAIN CONSOLE HANDLE (windows.h)");
		return;
	}

	COORD row_Size = { COLS,1 };
	COORD upper_left_Coord = { 0,0 };

	CHAR_INFO *curRow_Buffer = (CHAR_INFO*) calloc(COLS, sizeof(CHAR_INFO));
	for (uhex row=0; row<ROWS; row+=2) {
		SMALL_RECT curRow_Region = { 1,1+(row>>1), COLS,1+(row>>1) };
		for (uhex col=0; col<COLS; ++col) {
			if (row == 0) {
				curRow_Buffer[col].Attributes = 0b0111; // white (combines red, green, and blue parts)
			}

			cell8 top_cell = BIT(grid[((COLS>>3)*row + (col>>3))], col%8, 8);
			if (row == (ROWS-1)) {
				curRow_Buffer[col].Char.UnicodeChar = ((top_cell) ? alive_cell_top : dead_cell_full);
				continue;
			}
			cell8 bottom_cell = BIT(grid[((COLS>>3)*(row+1) + (col>>3))], col%8, 8);

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

	if (paused) {
		COORD note_Size = { 6, 1 };
		SMALL_RECT note_Region = { (COLS>>1)-2,(ROWS>>1)+1, (COLS>>1)+4,(ROWS>>1)+1 };
		CHAR_INFO note_Buffer[6] = {};
		const wchar_t note_display[6] = L"PAUSED";
		for (int i=0; i<6; ++i) {
			note_Buffer[i].Char.UnicodeChar = note_display[i];
			note_Buffer[i].Attributes = 0b0111;
		}

		WriteConsoleOutputW(
				hConsole,
				note_Buffer,
				note_Size,
				upper_left_Coord,
				&note_Region);
	}	

	/*	
	clock_t end = clock();
	double elapsed = (double) (end-start) / CLOCKS_PER_SEC;
	wprintf(L"\x1b[1;1f%.7lf seconds", elapsed);
	*/
}
#else
// not windows version of the print_grid function
void print_grid(cell8* grid) {
	//clock_t start = clock();
	wprintf(L"\x1b[2;2f\x1b[?25l");
	// we handle the rows in pairs because it makes the cells look more square
	for (uhex row = 0; row < ROWS; row += 2) {
		for (uhex col = 0; col < COLS; ++col) {
			cell8 top_cell = BIT(grid[((COLS>>3)*row + (col>>3))], col%8, 8);
			if (row == (ROWS-1)) {
				wprintf(L"%lc", ((top_cell==0) ? dead_cell_full : alive_cell_top));
				continue;
			}
			cell8 bottom_cell = BIT(grid[((COLS>>3)*(row+1) + (col>>3))], col%8, 8);
			if (top_cell && bottom_cell) {
				wprintf(L"%lc", alive_cell_full);
			} else if (top_cell != bottom_cell) {
				wprintf(L"%lc", ((top_cell) ? alive_cell_top : alive_cell_bottom));
			} else {
				wprintf(L"%lc", dead_cell_full);
			}
		}
		wprintf(L"\x1b[1B\x1b[%uD", COLS);
	}
	wprintf(L"\n");

	if (paused) {
		wprintf(L"\x1b[%lu;%lufPAUSED", 
				(ROWS>>1)+2,
				(COLS>>1)-1);
	}

	wprintf(L"\x1b[%lu;1f\x1b[?25h", (ROWS>>1)+3);

	fflush(stdout);

	/*clock_t end = clock();
	double elapsed = (double) (end-start)/CLOCKS_PER_SEC;
	wprintf(L"\x1b[1;1f%.7lf seconds", elapsed);
	*/
}
#endif
/* Rules:
 * <2: dies
 * 2 or 3: lives
 * >3: dies
 * dead cell with 3: becomes alive
 */
cell8* update(cell8* grid) {
	cell8* new_grid = (cell8*) calloc(GRID_EL_N, sizeof(cell8));

	// we'll treat any neighboring "cell" outside the grid boundaries as dead
	for (uhex i=0; i < (N_CELLS); ++i) {
		ubyte num_neighbors = 0;
		// cur cell: BIT(grid[i>>3], i%8, 8)
		uhex cell_col = i%COLS;
		uhex cell_row = i/COLS;
		if (cell_col > 0) {
			if (cell_row > 0) {
				num_neighbors += BIT(grid[(i-1-COLS)>>3], (i-1-COLS)%8, 8);
			}
			if (cell_row < (ROWS-1)) {
				num_neighbors += BIT(grid[(i-1+COLS)>>3], (i-1+COLS)%8, 8);
			}
			num_neighbors += BIT(grid[(i-1)>>3], (i-1)%8, 8);
		}
		if (cell_col < (COLS-1)) {
			if (cell_row > 0) {
				num_neighbors += BIT(grid[(i+1-COLS)>>3], (i+1-COLS)%8, 8);
			}
			if (cell_row < (ROWS-1)) {
				num_neighbors += BIT(grid[(i+1+COLS)>>3], (i+1+COLS)%8, 8);
			}
			num_neighbors += BIT(grid[(i+1)>>3], (i+1)%8, 8);
		}
		if (cell_row > 0) {
			num_neighbors += BIT(grid[(i-COLS)>>3], (i-COLS)%8, 8);
		}
		if (cell_row < (ROWS-1)) {
			num_neighbors += BIT(grid[(i+COLS)>>3], (i+COLS)%8, 8);
		}

		if ((BIT(grid[i>>3], i%8, 8) && num_neighbors >= 2 && num_neighbors <= 3) ||
			(!BIT(grid[i>>3], i%8, 8) && num_neighbors == 3)) {
			SETBIT(new_grid[i>>3], i%8, 8);
		}
	}

	return new_grid;
}

#ifdef _WIN32
DWORD WINAPI poll_input_thread() {
	char ch;

	while (1) {
		if (kbhit()) {
			ch = getch();
			if (ch == 'q' || ch == 'Q') {
				break;
			} else if (ch == 'p' || ch == 'P') {
				paused ^= 1;
				just_unpaused = paused^1;
				just_paused = paused;
			} else if (ch == 'n' || ch == 'N') {
				next_gen = 1;
			}

			__sync_synchronize();
		}
	}

	ready_to_quit = 1;
	__sync_synchronize();

	return 0;
}
#else
void* poll_input_thread() {
	struct termios old_term, new_term;
	tcgetattr(STDIN_FILENO, &old_term);
	new_term = old_term;
	new_term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
	
	char ch;
	
	while (1) {
		if (read(STDIN_FILENO, &ch, 1) == 1) {
			if (ch == 'q' || ch == 'Q') {
				break;
			} else if (ch == 'p' || ch == 'P') {
				paused ^= 1;
				just_unpaused = paused^1;
				just_paused = paused;
			} else if (ch == 'n' || ch == 'N') {
				next_gen = 1;
			}

			__sync_synchronize();
		}
	}

	ready_to_quit = 1;
	__sync_synchronize();
	
	tcsetattr(STDIN_FILENO, TCSANOW, &old_term);

	pthread_exit(NULL);
	
	return NULL;
}
#endif

int main(int argc, char *argv[]) {
	// define the locale to support Windows and other platforms (since Windows wants a different value for some reason)
	cp_set_unicode_locale();
	fwide(stdout, 1);
	

	if (argc > 1) { // arguments should be in "COLSxROWS"
		for (size_t i=0; i<strlen(argv[1]); ++i) {
			if (argv[1][i] == 'x') {
				char* tmp2 = (char*) calloc(i+1, sizeof(char));
				char* tmp3 = (char*) calloc(strlen(argv[1])-1, sizeof(char));
				memcpy(tmp2, argv[1], i);
				for (size_t j=(i+1); j<strlen(argv[1]); ++j) {
					tmp3[j-i-1] = argv[1][j];
				}

				/* the expression below rounds it up to the nearest multiple of 8 (or 2 for rows) because 
				 * i can't be bothered to fix my code to work with dimensions that aren't multiples of 8
				 */
				ROWS = (atoi(tmp3)+1) & ~1; 
				COLS = (atoi(tmp2)+7) & ~7;
				CC = COLS>>3;
				N_CELLS = ROWS*COLS;
				GRID_EL_N = N_CELLS>>3;
			}
		}
	}
	
	printf("finished checking command line arguments\n");

	cell8* grid = (cell8*) calloc(GRID_EL_N, sizeof(cell8));
	
	// initialize the default cell positions (this can be changed, but i haven't made an easier way to do that just yet)
	grid[ 4*CC+4] = 0b00100000;
	grid[ 5*CC+2] = 0b00001000;
	grid[ 5*CC+4] = 0b10000000;
	grid[ 9*CC+2] = 0b10100000;
	grid[ 9*CC+3] = 0b00000100;
	grid[10*CC+3] = 0b00001100;
	grid[11*CC+2] = 0b00000010;
	grid[11*CC+3] = 0b00100000;
	grid[11*CC+4] = 0b01000000;
	grid[12*CC+2] = 0b00010000;
	grid[12*CC+3] = 0b00010010;
	grid[13*CC+2] = 0b00010000;
	grid[13*CC+3] = 0b11000000;
	grid[13*CC+4] = 0b11000000;
	grid[14*CC+2] = 0b00010011;
	grid[14*CC+4] = 0b00100000;
	grid[15*CC+2] = 0b00000100;
	grid[15*CC+3] = 0b00100000;
	grid[16*CC+3] = 0b00001000;
	grid[16*CC+4] = 0b00000100;
	grid[17*CC+4] = 0b01000000;
	grid[18*CC+2] = 0b00000111;
	grid[19*CC+3] = 0b00000010;
	grid[20*CC+3] = 0b00011000;
	grid[22*CC+2] = 0b00000100;
	grid[22*CC+3] = 0b10000000;
	
	wprintf(L"\x1b[2J");
	print_grid(grid);
	
	#ifdef _WIN32
	DWORD dwPollingThread;
	//HANDLE hPollingThread = 
		CreateThread(
			NULL,
			0,
			poll_input_thread,
			NULL,
			0,
			&dwPollingThread);
	#else
	pthread_t input_polling_thread;
	pthread_create(&input_polling_thread, NULL, poll_input_thread, NULL);
	#endif
	
	printbox(
			COLS+2,(ROWS>>1)+2,
			0,0,
			W1T1_BORDERCHARS,
			1);

	//int was_too_small = 0;
	
	while (1) {
		__sync_synchronize();
		if (ready_to_quit) {
			wprintf(L"\x1b[%lu;1f", (ROWS>>1)+3);
			break;
		}

		// check if terminal size is too small
		/*uhex rows, cols;
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
						1);
			}
			print_grid(grid);
		}*/

		// generate the new grid and update the old one
		__sync_synchronize();
		
		if (just_unpaused) {
			printbox(
					COLS+2,(ROWS>>1)+2,
					0,0,
					W1T1_BORDERCHARS,
					0);
			just_unpaused = 0;
		}
		if (just_paused) {
			print_grid(grid);
			just_paused = 0;
		}
		__sync_synchronize();

		if (!paused) {
			print_grid(grid);
			cell8* new_grid = update(grid);
			memcpy(grid, new_grid, GRID_EL_N);
			free(new_grid);
		} else if (next_gen) {
			cell8* new_grid = update(grid);
			memcpy(grid, new_grid, GRID_EL_N);
			free(new_grid);
			next_gen = 0;
			print_grid(grid);
		}

		#ifdef _WIN32
		sleepns(1000000000/20 - 10000000);
		#else
		sleepns(1000000000/20); // sleeps to keep a framerate (`sleepns` takes nanoseconds)
		#endif
	}

	free(grid);

	return 0;
}
