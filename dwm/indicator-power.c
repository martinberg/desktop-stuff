#include <stdlib.h>
#include <string.h>

#include "dwm.h"
#include "indicator.h"

#define MENU_WIDTH 128

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
} menu={0};

typedef struct Icon Icon;
struct Icon {
	int percentage;
	const char *str;
};

static Icon icon[] = {
	{10, "\uf212"},
	{40, "\uf215"},
	{80, "\uf214"},
	{101, "\uf213"},
};

static void menu_open(Indicator *indicator) {
	menu.selected=-1;
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*1;
	menu.window=XCreateSimpleWindow(dpy, root, 
		menu.x, menu.y, menu.w, menu.h,
		1, dc.sel[ColBorder], dc.norm[ColBG]
	);
	menu.gc=XCreateGC(dpy, menu.window, 0, 0);
	XSelectInput(dpy, menu.window, ExposureMask|ButtonPressMask|PointerMotionMask);
	XDefineCursor(dpy, menu.window, cursor[CurNormal]);
	XMapRaised(dpy, menu.window);
}

static void menu_close() {
	XFreeGC(dpy, menu.gc);
	XUnmapWindow(dpy, menu.window);
	XDestroyWindow(dpy, menu.window);
}

int indicator_power_init(Indicator *indicator) {
	FILE *cap, *stat;
	
	if(!(cap = fopen("/sys/class/power_supply/BAT0/capacity", "r")))
		return -1;
	if(!(stat = fopen("/sys/class/power_supply/BAT0/status", "r"))) {
		fclose(cap);
		return -1;
	}
	fclose(stat);
	fclose(cap);
	return 0;
}

void indicator_power_update(Indicator *indicator) {
	char percentage[10];
	char status[20];
	char *nl;
	int i, p;
	
	FILE *cap = fopen("/sys/class/power_supply/BAT0/capacity", "r");
	FILE *stat = fopen("/sys/class/power_supply/BAT0/status", "r");
	
	fread(percentage, 10, 1, cap);
	fread(status, 20, 1, stat);
	if((nl = strchr(percentage, '\n')))
		*nl = 0;
	if((nl = strchr(status, '\n')))
		*nl = 0;
	if(!strcmp(status, "Charging"))
		sprintf(indicator->text, " \uf211 %s%% ", percentage);
	else {
		p = atoi(percentage);
		for(i = 0; i < sizeof(icon)/sizeof(Icon); i++) {
			if(p < icon[i].percentage)
				break;
		}
		if(p < icon[0].percentage)
			sprintf(indicator->text, " <span foreground=\"red\">%s %s%%</span> ", icon[0].str, percentage);
		else
			sprintf(indicator->text, " %s %s%% ", icon[i].str, percentage);
	}
	
	fclose(cap);
	fclose(stat);
}

void indicator_power_expose(Indicator *indicator, Window window) {
	char status[20];
	char *nl;
	
	if(window!=menu.window)
		return;
	
	FILE *stat = fopen("/sys/class/power_supply/BAT0/status", "r");
	fread(status, 20, 1, stat);
	if((nl = strchr(status, '\n')))
		*nl = 0;
	indicator_draw_text(menu.window, menu.gc, 0, 0, MENU_WIDTH, bh, dc.norm, status, False);
	fclose(stat);
}

Bool indicator_power_haswindow(Indicator *indicator, Window window) {
	return menu.window==window?True:False;
}

void indicator_power_mouse(Indicator *indicator, XButtonPressedEvent *ev) {
	if(ev->type != ButtonPress) {
		return;
	}
	if(ev->window==menu.window) {
		indicator_power_expose(indicator, ev->window);
		return;
	}
	switch(ev->button) {
		case Button1:
		case Button3:
			if((indicator->active=!indicator->active))
				menu_open(indicator);
			else
				menu_close();
			return;
		case Button4:
			break;
		case Button5:
			break;
	}
	if(indicator->active)
		indicator_power_expose(indicator, menu.window);
}
