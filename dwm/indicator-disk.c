#include <sys/statvfs.h>

#include "dwm.h"
#include "indicator.h"

#define MENU_WIDTH 200

static char *discs[]={
	"/",
	"/media/data",
};

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
} menu={0};

static void menu_open(Indicator *indicator) {
	menu.selected=-1;
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*(sizeof(discs)/sizeof(char *));
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
int indicator_disk_init(Indicator *indicator) {
	return 0;
}

void indicator_disk_update(Indicator *indicator) {
	struct statvfs sbuf;
	unsigned long p;
	
	if(statvfs("/", &sbuf) < 0)
		return;
	
	p = 100*(sbuf.f_blocks-sbuf.f_bavail)/sbuf.f_blocks;
	
	if(p >= 95)
		sprintf(indicator->text, " <span foreground=\"red\">\uf0a0 %lu%%</span> ", p);
	else
		sprintf(indicator->text, " \uf0a0 %lu%% ", p);
}

void indicator_disk_expose(Indicator *indicator, Window window) {
	int i;
	char buf[256];
	struct statvfs sbuf;
	
	if(window!=menu.window)
		return;
	
	for(i=0; i<(sizeof(discs)/sizeof(char *)); i++) {
		if(statvfs(discs[i], &sbuf)>=0) {
			sprintf(buf, " %s %lu%% ", discs[i], 100*(sbuf.f_blocks-sbuf.f_bavail)/sbuf.f_blocks);
			indicator_draw_text(menu.window, menu.gc, 0, i*bh, MENU_WIDTH, bh, dc.norm, buf, False);
		}
	}
}

Bool indicator_disk_haswindow(Indicator *indicator, Window window) {
	return menu.window==window?True:False;
}

void indicator_disk_mouse(Indicator *indicator, XButtonPressedEvent *ev) {
	if(ev->type != ButtonPress) {
		return;
	}
	if(ev->window==menu.window) {
		indicator_disk_expose(indicator, ev->window);
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
		indicator_disk_expose(indicator, menu.window);
}
