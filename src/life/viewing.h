#ifndef VIEWING_H
#define VIEWING_H

#include <stdint.h>

void display_paused_notif(void);
void print_grid(uint8_t *grid);
void help_screen(void);
void close_help_screen(uint8_t *grid);

#endif /* VIEWING_H */
