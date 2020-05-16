#ifndef WINDOW_H
#define WINDOW_H

void scr_init();
void scr_update(const char * info);
void scr_clean();
void scr_print_bottom(int row, int col, const char *fmt, ...);
void scr_print_middle(const char *fmt, ...);

#define WITH_NCURSES

#endif /* WINDOW_H */
