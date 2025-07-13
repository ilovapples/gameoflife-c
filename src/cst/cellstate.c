#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>

#define BIT(i) (1<<(i))
#define FLAG_SET(f) ((runtime_flags & f) != 0)
#define SET_FLAG(f) (runtime_flags |= f)
#define IFDBG if (FLAG_SET(DEBUG))


#define MAX_DIM_STR_LEN 5
#define MAX_FIRST_LINE_LEN (2*MAX_DIM_STR_LEN+4)
#define MAX_FILES_AT_ONCE 128
#define MAX_FILE_SIZE 16781322
#define MAX_OUTPUT_BIT_SIZE 16777248
#define MAX_OUTPUT_BYTE_SIZE 2097156
/* 16781322 = 2^12 * 2^12 + 2^12 + 8 + 2 (the max size of an input file)
 * 16 bits for row and col sizes (valid is between 1 and 4095 for both)
 *
 * (if you want to go higher than 4096, you can. I didn't add any checks to make sure the dimensions weren't gigantic.
 *  however, I doubt anyone has a display so large they can show between 4096 and (2^16)-1=65535 characters vertically 
 *  and horizontally, so there's not much use in that. although, the MAX_FILE_SIZE, MAX_OUTPUT_BIT_SIZE, and MAX_OUTPUT_BYTE_SIZE
 *  were precalculated with the assumption that 4095 would be the maximum value in both dimensions, not that they're all checked)
 * 
 * 4096^2 = (2^12)^2 = 2^24 = 16777216, then add 32 bits (4 bytes) at the start to store the row and col values (16 bits each)
 */

struct FileData {
	char *path;
	char *contents;
	size_t cont_len;
	char *cont_ptr;
	uint8_t *output_bin;
	size_t output_byte_size;
	uint16_t n_cols;
	uint16_t n_rows;
	bool failed;
}; // should be about 48 bytes

enum FLAGS {
	DEBUG = BIT(0),
	QUIET = BIT(1),
	FORCE = BIT(2)
};
static uint8_t runtime_flags = 0;

bool path_has_extension(struct FileData *file, char **path_ext_sep_out)
{
	// i'm too lazy so this function only knows if you're pointing to a directory
	// if the path ends in a '/' or '\'
	size_t pathin_len = strlen(file->path);
	
	if (file->path[pathin_len-1] == '/' || 
	    file->path[pathin_len-1] == '\\') {
		file->failed = true;
		return 0;
	}
	
	char *last_fslash  = NULL,
	     *last_bslash  = NULL,
	     *last_ext_sep = NULL;

	char *pathin_end = file->path + pathin_len;
	while (--pathin_end >= file->path) {
		if (*pathin_end == '/' && last_fslash == NULL)
			last_fslash = pathin_end;
		else if (*pathin_end == '\\' && last_bslash == NULL)
			last_bslash = pathin_end;
		else if (*pathin_end == '.' && last_ext_sep == NULL)
			last_ext_sep = pathin_end;
	}
	
	char *last_slash = (last_fslash > last_bslash) ? last_fslash : last_bslash;

	*path_ext_sep_out = last_ext_sep;
	
	return (last_ext_sep > last_slash);
}

int32_t parse_cmd_args_for_paths(struct FileData files[MAX_FILES_AT_ONCE], int32_t argc, char **argv)
{
	uint32_t num_paths = 0;
	// reads args from right to left, it's probably fine because the order doesn't actually matter
	while (--argc)
		if (argv[argc][0] != '-')
			files[num_paths++].path = argv[argc];
		else {
			if (argv[argc][1] == '-') { // if the arg starts with "--" (long arg)
				if (strcmp(argv[argc]+2, "debug") == 0)
					SET_FLAG(DEBUG);
				else if (strcmp(argv[argc]+2, "quiet") == 0)
					SET_FLAG(QUIET);
				else if (strcmp(argv[argc]+2, "force") == 0)
					SET_FLAG(FORCE);
			} else { // if it starts with "-" (short arg or combined short args)
				char *argp = argv[argc]+1;
				size_t arg_len = strlen(argp);
				while (--arg_len)
					switch (argp[arg_len]) {
						case 'd': SET_FLAG(DEBUG);
							    break;
						case 'q': SET_FLAG(QUIET);
							    break;
						case 'f': SET_FLAG(FORCE);
							    break;
					}
			}
		}

	if (num_paths == 0 || num_paths > MAX_FILES_AT_ONCE) {
		return 0;
	}

	if (!FLAG_SET(QUIET)) {
		printf("Compiling %d file%s.\n", num_paths, (num_paths==1) ? "" : "s");

		printf("\nCompiling the following file(s): \n");
		for (uint32_t i=0; i < num_paths; ++i) {
			printf(" - '%s'\n", files[i].path);
		}
	}

	return num_paths;
}

int main(int32_t argc, char **argv)
{
	struct FileData files[MAX_FILES_AT_ONCE] = {0};
	struct FileData *curfile;

	uint32_t num_failed = 0;

	uint32_t num_paths = parse_cmd_args_for_paths(files, argc, argv);
	if (num_paths == 0) {
		fprintf(stderr, "usage: cst [options] <file_path (n<=128)>...\n\n"

				    "Options:\n"
				    "  -d, --debug      Enable debug output\n"
				    "  -q, --quiet      Disable all non-debug, non-interactive output\n"
				    "  -f, --force      Don't prompt to overwrite existing binary\n");
		return EXIT_FAILURE;
	}

	// read the contents of the files into the `file_contents` buffer
	// TODO: use fread to make this bit less ugly
	for (curfile = files; curfile < files + num_paths; curfile++) {
		FILE *fp = fopen(curfile->path, "rb");
		if (fp == NULL) {
			fprintf(stderr, "Failed to open file at path '%s' (check if it's a directory)\n", curfile->path);
			curfile->failed = true;
			num_failed++;
			continue;
		}

		char first_line_buf[MAX_FIRST_LINE_LEN + 1]; // the first line should be up to around 12-13 characters at max
		fgets(first_line_buf, MAX_FIRST_LINE_LEN + 1, fp);
		rewind(fp);

		// get number of columns and rows
		curfile->n_cols = 0;
		curfile->n_rows = 0;

		char *cp;

		for (cp = first_line_buf; cp < first_line_buf + (MAX_FIRST_LINE_LEN + 1); cp++) {
			if (*cp == ' ' || *cp == 'x') {
				// the (n+7)&~7 returns n rounded up to the nearest multiple of 8
				curfile->n_cols = ((uint16_t) strtoul(first_line_buf, NULL, 10) + 7) & ~7;
				curfile->n_rows = ((uint16_t) strtoul(cp+1, NULL, 10)           + 1) & ~1;
			} else if (*cp == '\n') {
				break;
			}
		}

		size_t curfile_cont_max = (cp - first_line_buf) + (curfile->n_cols+2)*curfile->n_rows;
		curfile->contents = (char *) malloc(curfile_cont_max + 1);
		if (curfile->contents == NULL) {
			fprintf(stderr, "Failed to allocate memory for a buffer to read the contents of '%s'\n", curfile->path);
			curfile->failed = true;
			num_failed++;
			fclose(fp);
			continue;
		}

		size_t bytes_read = fread(curfile->contents, 1, curfile_cont_max + 1, fp);
		if (bytes_read == 0) {
			fprintf(stderr, "Failed to read contents of '%s' (check if it's corrupted or truncated somehow)\n", curfile->path);
			curfile->failed = true;
			num_failed++;
			free(curfile->contents);
			fclose(fp);
			continue;
		}
			
		curfile->contents[bytes_read] = '\0';
		curfile->cont_len = bytes_read;
		
		curfile->cont_ptr = curfile->contents + (cp - first_line_buf);

		fclose(fp);
	}

	if (FLAG_SET(DEBUG)) {
		for (curfile = files; curfile < files + num_paths; curfile++) {
			printf("\ncontents of '%s': \n%s", curfile->path, curfile->contents);
		}
	}

	// i can't be bothered to implement proper error handling, so we're 
	// just going to assume every file has valid contents for the format
	for (curfile = files; curfile < files + num_paths; curfile++) {
		if (curfile->failed) continue;

		
		// here: open file to be written later using a file extension that's just the original filename stripped of its extension + .bin
		char *extension_sep;
		bool has_extension = path_has_extension(curfile, &extension_sep);

		char *output_bin_path;
		if (!has_extension) {
			output_bin_path = (char *) malloc(strlen(curfile->path) + 4 + 1);
			if (output_bin_path == NULL) {
				fprintf(stderr, "Failed to allocate memory for the output path for the file originally at '%s'\n", curfile->path);
				curfile->failed = true;
				num_failed++;
				free(curfile->contents);
				continue;
			}

			strcpy(output_bin_path, curfile->path);
			strcat(output_bin_path, ".bin");
		} else {
			ptrdiff_t path_base_len = (extension_sep - curfile->path);
			output_bin_path = (char *) malloc((size_t)path_base_len + 4 + 1);
			if (output_bin_path == NULL) {
				fprintf(stderr, "Failed to allocate memory for the output path for the file originally at '%s'\n", curfile->path);
				curfile->failed = true;
				num_failed++;
				free(curfile->contents);
				IFDBG printf("[ERROR] debug message #6 for '%s' [ERROR]\n", curfile->path);
				continue;
			}

			memcpy(output_bin_path, curfile->path, (size_t)path_base_len);
			strcpy(output_bin_path + path_base_len, ".bin");
		}

		// initialize curfile->output_bin
		curfile->output_byte_size = 
			2*sizeof(uint16_t) + curfile->n_cols*curfile->n_rows / 8;

		curfile->output_bin = (uint8_t *) calloc(curfile->output_byte_size, 1);
		if (curfile->output_bin == NULL) {
			fprintf(stderr, "Failed to allocate memory for the output binary extracted from '%s' intended to be written to '%s'",
					curfile->path, output_bin_path);
			curfile->failed = true;
			num_failed++;
			free(curfile->contents);
			continue;
		}
		
		uint8_t *binp = curfile->output_bin;
		
		memcpy(binp, &curfile->n_rows, sizeof(uint16_t)); 
		binp += sizeof(uint16_t);
		memcpy(binp, &curfile->n_cols, sizeof(uint16_t));
		binp += sizeof(uint16_t);

		// read through the rest of the file and set the values
		int8_t bit_num = 0; // counts the bit being set (will be incremented mod 8)

		register char ch;
		register uint8_t curbyte = 0x00;

		while ((curfile->cont_ptr < curfile->contents + curfile->cont_len)
		    && (binp < curfile->output_bin + curfile->output_byte_size)) {
			ch = *curfile->cont_ptr++;
			// this code should just completely ignore any character that isn't a '-' or '#'

			if (ch == '-' || ch == '#') {
				curbyte |= ((ch=='#') << (7-bit_num));
				bit_num = (bit_num + 1) % 8;
				if (bit_num == 0) {
					*binp++ = curbyte;
					curbyte = 0x00;
				}
			}
		}

		if (bit_num != 0) *binp++ = curbyte;

		// check if output bin file already exists
		if (!FLAG_SET(FORCE)) {
			FILE *tfp;
			if ((tfp = fopen(output_bin_path, "r")) != NULL) {
				printf("Overwrite existing file at '%s' (Y/n)?: ", output_bin_path);
				char in_buf[5];
				fgets(in_buf, 5, stdin);
				if (tolower(*in_buf) == 'n') {
					// clean up malloced arrays
					free(curfile->output_bin);
					free(curfile->contents);
					free(output_bin_path);
					continue;
				}
				fclose(tfp);
			}
		}

		// open output bin file and write the binary
		if (!FLAG_SET(QUIET)) printf("Writing %zu bytes to '%s'...", curfile->output_byte_size, output_bin_path);

		FILE *binfp = fopen(output_bin_path, "wb");
		if (binfp == NULL) {
			fprintf(stderr, "Failed to open output binary file at path '%s' for original file '%s'", output_bin_path, curfile->path);
			curfile->failed = true;
			num_failed++;
			free(curfile->output_bin);
			free(curfile->contents);
			free(output_bin_path);
			continue;
		}
		fwrite(curfile->output_bin, 1, curfile->output_byte_size, binfp);
		fclose(binfp);

		if (!FLAG_SET(QUIET)) printf("DONE\n");

		// free the binary buffer at the end of the loop
		free(curfile->output_bin);
		free(curfile->contents);
		free(output_bin_path);
	}

	if (num_failed > 0)
		fprintf(stderr, "%d files failed to convert.\n", num_failed);

	return EXIT_SUCCESS;
}
