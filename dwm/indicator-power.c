#include <stdlib.h>
#include <string.h>

#include "dwm.h"

#define MENU_WIDTH 128

static FILE *acpi_capacity;
static FILE *acpi_status;

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
} menu={0};

static void draw_text(int x, int y, int w, int h, XftColor col[ColLast], const char *text) {
	char buf[256];
	int i, texth, len, olen;
	XftDraw *d;
	
	XSetForeground(dpy, menu.gc, col[ColBG].pixel);
	XFillRectangle(dpy, menu.window, menu.gc, x, y, w, h);
	if(!text)
		return;
	olen=strlen(text);
	texth=dc.font.ascent + dc.font.descent;
	y+=(h/2)-(texth/2)+dc.font.ascent;
	x+=(texth/2);
	/* shorten text if necessary */
	for(len=MIN(olen, sizeof buf); len&&textnw(text, len)>w-texth; len--);
	if(!len)
		return;
	memcpy(buf, text, len);
	if(len<olen)
		for(i=len; i&&i>len-3; buf[--i]='.');

	d=XftDrawCreate(dpy, menu.window, DefaultVisual(dpy, screen), DefaultColormap(dpy,screen));
	XftDrawStringUtf8(d, &col[ColFG], dc.font.xfont, x, y, (XftChar8 *) buf, len);
	XftDrawDestroy(d);
}

static void menu_open(Indicator *indicator) {
	menu.selected=-1;
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*1;
	menu.window=XCreateSimpleWindow(dpy, root, 
		menu.x, menu.y, menu.w, menu.h,
		1, dc.sel[ColBorder].pixel, dc.norm[ColBG].pixel
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
	if(!(acpi_capacity = fopen("/sys/class/power_supply/BAT0/capacity", "r")))
		return -1;
	if(!(acpi_status = fopen("/sys/class/power_supply/BAT0/status", "r"))) {
		fclose(acpi_capacity);
		return -1;
	}
	return 0;
}

void indicator_power_update(Indicator *indicator) {
	char percentage[10];
	char *nl;
	fseek(acpi_capacity, 0, SEEK_SET);
	fread(percentage, 10, 1, acpi_capacity);
	if((nl = strchr(percentage, '\n')))
		*nl = 0;
	sprintf(indicator->text, " ⚡ %s%%", percentage);
}

void indicator_power_expose(Indicator *indicator, Window window) {
	char status[20];
	char *nl;
	
	if(window!=menu.window)
		return;
	
	fseek(acpi_status, 0, SEEK_SET);
	fread(status, 20, 1, acpi_status);
	if((nl = strchr(status, '\n')))
		*nl = 0;
	draw_text(0, 0, MENU_WIDTH, bh, dc.norm, status);
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
