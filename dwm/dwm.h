#ifndef DWM_H
#define DWM_H

#include <time.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

typedef struct Indicator {
	Bool active;
	char text[64];
	int x;
	int width;
	int (*init)(struct Indicator *indicator);
	void (*update)(struct Indicator *indicator);
	void (*mouse)(struct Indicator *indicator, unsigned int button);
	struct Indicator *next;
} Indicator;

int textnw(const char *text, unsigned int len);

int indicator_time_init(Indicator *indicator);
void indicator_time_update(Indicator *indicator);
void indicator_time_mouse(Indicator *indicator, unsigned int button);

int indicator_music_init(Indicator *indicator);
void indicator_music_update(Indicator *indicator);
void indicator_music_mouse(Indicator *indicator, unsigned int button);

#endif
