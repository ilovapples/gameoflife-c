#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>

#include "runtime_flags.h"
#include "big_header.h"
#include "arg_parse.h"

extern char *PROG_NAME;

extern uint16_t ROWS, COLS;
extern uint8_t runtime_flags,
	 	   bin_loaded;

extern uint8_t *grid;


void arg_parse(int32_t argc, char **argv)
{
	PROG_NAME = argv[0];

	for (int32_t arg_n = 0; ++arg_n < argc;) {
		if (argv[arg_n][0] == '-') {
			if (argv[arg_n][1] == '-') {
				// dimensions=
				// 123456789AB
				if (strncmp(argv[arg_n]+2, "dimensions=", 11) == 0)
					get_dims_from_str(argv[arg_n]+2 + 11);
				else if (strncmp(argv[arg_n]+2, "binary=", 7) == 0)
					load_bin_file(argv[arg_n]+2 + 7);
				else if (strcmp(argv[arg_n]+2, "debug") == 0)
					SET_FLAG(DEBUG);
				else if (strcmp(argv[arg_n]+2, "help") == 0)
					print_usage_msg();
			} else {
				char *argp = argv[arg_n];
				bool end_read = false;
				while (*argp++ && !end_read) {
					switch (*argp) {
						case '?':
						case 'h':
							print_usage_msg();
							break;
						case 'd': 
							SET_FLAG(DEBUG);
							break;
						case 'B':
							end_read = true;
							if (*(argp+1) == '\0' && arg_n < argc-1)
								load_bin_file(argv[arg_n+1]);
							else if (*(argp+1) != '\0')
								load_bin_file(argp+1);
							else
								print_usage_msg();
							break;
						case 'D':
							end_read = true;
							if (*(argp+1) == '\0' && arg_n < argc-1)
								// pass in the next argument in the list if this argument is over and
								// isn't the last in the list
								get_dims_from_str(argv[arg_n+1]);
							else if (*(argp+1) != '\0')
								// pass in this argument because the 'D' isn't the end
								get_dims_from_str(argp+1);
							else
								// print the usage message because the user didn't put the dimensions after 
								// the "-D"
								print_usage_msg();
							break;
					}
				}
			}
		}
	}
}

void print_usage_msg(void)
{
	wprintf(L"usage: %s [options]\n\n"

		  L"{-B path_to_bin | -Bpath_to_bin | \n"
		  L" --binary=path_to_bin }         Load a binary file as the state of the grid\n"
		  L"{-D COLSxROWS | -DCOLSxROWS | \n"
		  L" --dimensions=COLSxROWS}        Sets the dimensions of the grid and viewport\n"
		  L"-d, --debug                     Enables debug output\n"
		  L"-h, --help                      Displays this help message\n\n"

		  L"(press 'h' or '?' in the simulator to open the keybinds menu)\n"
		  , PROG_NAME);
	exit(1);
}

void get_dims_from_str(char *s)
{
	for (char *p = s; *p != '\0'; p++) {
		if (*p == 'x' && *(p+1) != '\0') {
			// the (n+7)&~7 returns n rounded up to the nearest multiple of 8
			uint16_t cols = ((uint16_t) strtoul(s,   NULL, 10) + 7) & ~7;
			uint16_t rows = ((uint16_t) strtoul(p+1, NULL, 10) + 1) & ~1;

			set_dims(rows, cols);
			return;
		}
	}
	
	print_usage_msg();
}

void load_bin_file(char *path)
{
	FILE *fp = fopen(path, "rb");
	if (fp == NULL) {
		fprintf(stderr, "failed to open binary file at '%s' (make sure it exists)\n", path);
		exit(2);
	}

	uint16_t rows, cols;
	fread(&rows, 1, 2, fp);
	fread(&cols, 1, 2, fp);

	set_dims(rows, cols);

	const uint16_t CONTENTS_SIZE = COLS * ROWS;

	grid = (uint8_t *) calloc(CONTENTS_SIZE, 1);
	if (grid == NULL) {
		fprintf(stderr, "failed to allocate space for the grid.");
		exit(1);
	}

	fread(grid, 1, CONTENTS_SIZE, fp);

	bin_loaded = true;

	fclose(fp);
}
