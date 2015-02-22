#ifndef __INDICATOR_H_
#define __INDICATOR_H_

void indicator_draw_text(Window window, GC gc, int x, int y, int w, int h, unsigned long col[ColLast], const char *text, Bool markup);

#endif
