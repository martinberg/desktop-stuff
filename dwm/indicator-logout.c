#include "dwm.h"
#include "dbus.h"
#include "power.h"

#define MENU_WIDTH 128
#define QUESTION_WIDTH 300
#define BUTTON_W 40

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
	int button;
} menu={0};

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
	int selected;
	const char *text;
	void (*action)();
} question = {0};

static void menu_open(Indicator *indicator) {
	menu.selected=-1;
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*4;
	menu.window=XCreateSimpleWindow(dpy, root, 
		menu.x, menu.y, menu.w, menu.h,
		1, dc.sel[ColBorder].pixel, dc.norm[ColBG].pixel
	);
	menu.gc=XCreateGC(dpy, menu.window, 0, 0);
	XSelectInput(dpy, menu.window, ExposureMask|ButtonPressMask|PointerMotionMask);
	XDefineCursor(dpy, menu.window, cursor[CurNormal]);
	XMapRaised(dpy, menu.window);
}

static void question_open(Indicator *indicator) {
	question.selected=-1;
	question.x=selmon->mx + selmon->mw/2 - QUESTION_WIDTH/2;
	question.y=selmon->my + selmon->mh/2 -  bh*2;
	question.w=QUESTION_WIDTH;
	question.h=bh*4;
	question.window=XCreateSimpleWindow(dpy, root, 
		question.x, question.y, question.w, question.h,
		1, dc.sel[ColBorder].pixel, dc.norm[ColBG].pixel
	);
	question.gc=XCreateGC(dpy, question.window, 0, 0);
	XSelectInput(dpy, question.window, ExposureMask|ButtonPressMask|PointerMotionMask);
	XDefineCursor(dpy, question.window, cursor[CurNormal]);
	XMapRaised(dpy, question.window);
}

static void question_close() {
	XFreeGC(dpy, question.gc);
	XUnmapWindow(dpy, question.window);
	XDestroyWindow(dpy, question.window);
	question.window = 0;
}

static void menu_close() {
	XFreeGC(dpy, menu.gc);
	XUnmapWindow(dpy, menu.window);
	XDestroyWindow(dpy, menu.window);
}

static void draw_text(Window window, GC gc, int x, int y, int w, int h, XftColor col[ColLast], const char *text) {
	char buf[256];
	int i, texth, len, olen;
	XftDraw *d;

	XSetForeground(dpy, gc, col[ColBG].pixel);
	XFillRectangle(dpy, window, gc, x, y, w, h);
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

	d=XftDrawCreate(dpy, window, DefaultVisual(dpy, screen), DefaultColormap(dpy,screen));
	XftDrawStringUtf8(d, &col[ColFG], dc.font.xfont, x, y, (XftChar8 *) buf, len);
	XftDrawDestroy(d);
}

int indicator_logout_init(Indicator *indicator) {	
	return 0;
}

void indicator_logout_update(Indicator *indicator) {
	sprintf(indicator->text, " ☾ ");
}

void indicator_logout_expose(Indicator *indicator, Window window) {
	char buf[256];
	if(window == menu.window) {
		draw_text(menu.window, menu.gc, 0, 0, menu.w, bh, menu.selected==0?dc.sel:dc.norm, "Log out");
		draw_text(menu.window, menu.gc, 0, bh, menu.w, bh, menu.selected==1?dc.sel:dc.norm, "Suspend");
		draw_text(menu.window, menu.gc, 0, 2*bh, menu.w, bh, menu.selected==2?dc.sel:dc.norm, "Restart");
		draw_text(menu.window, menu.gc, 0, 3*bh, menu.w, bh, menu.selected==3?dc.sel:dc.norm, "Shut down");
	} else if(window == question.window) {
		snprintf(buf, 256, "Are you sure you want to %s?", question.text);
		draw_text(question.window, question.gc, 0, 0, question.w, bh*2, dc.norm, buf);
		draw_text(question.window, question.gc, question.w/2 - 64, bh*3, 64, bh, question.selected==0?dc.sel:dc.norm, "Yes");
		draw_text(question.window, question.gc, question.w/2, bh*3, 64, bh, question.selected==1?dc.sel:dc.norm, "No");
	}
}

Bool indicator_logout_haswindow(Indicator *in, Window window) {
	return window == menu.window || window == question.window;
}

void indicator_logout_mouse(Indicator *indicator, XButtonPressedEvent *ev) {
	if(ev->type != ButtonPress) {
		XMotionEvent *motev = (void *) ev;
		Window w = motev->window;
		int x = motev->x, y = motev->y;
		
		if(w == menu.window) {
			menu.selected=-1;
			if(x>=0&&y>=0&&x<menu.w&&y<menu.h) {
				menu.selected=y/bh;
			}
			indicator_logout_expose(indicator, menu.window);
		} else if(w == question.window) {
			question.selected=-1;
			if(y >= bh*3 && y < bh*4) {
				if(x >= question.w/2 && x < question.w/2 + 64) {
					question.selected = 1;
				} else if(x >= question.w/2 - 64 && x < question.w/2) {
					question.selected = 0;
				}
			}
			indicator_logout_expose(indicator, question.window);
		}
		
		return;
	}
	if(ev->window==menu.window && (ev->button & Button1)) {
		int item=ev->y/bh;
		const char *text[] = {
			"log out",
			"suspend",
			"restart",
			"shut down",
		};
		void (*action[])() = {
			power_logout,
			power_suspend,
			power_restart,
			power_shutdown,
		};
		
		if(item > 3)
			return;
		
		question.text = text[item];
		question.action = action[item];
		question_open(indicator);
		indicator->active = False;
		menu_close();
		
		return;
	} else if(ev->window == question.window && (ev->button & Button1)) {
		switch(question.selected) {
			case 0:
				question.action();
			case 1:
				question_close();
		}
		return;
	}
	
	switch(ev->button) {
		case Button1:
		case Button3:
			if(question.window > 0)
				return;
			
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
		indicator_logout_expose(indicator, menu.window);
}
