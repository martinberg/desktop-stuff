#include <dwm.h>
#include <indicator.h>

void indicator_draw_text(Window window, GC gc, int x, int y, int w, int h, unsigned long col[ColLast], const char *text, Bool markup) {
	char buf[512];
	int i, texth, len, olen;
	XftDraw *d;

	XSetForeground(dpy, gc, col[ColBG]);
	XFillRectangle(dpy, window, gc, x, y, w, h);
	if(!text)
		return;
	olen = strlen(text);
	texth = dc.font.ascent + dc.font.descent;
	x += texth/2;
	y += h/2 - texth/2;
	/* shorten text if necessary (this could wreak havoc with pango markup but fortunately
	   dc.w is adjusted to the width of the status text and not the other way around) */
	len = MIN(olen, sizeof buf);
	while(len) {
		if(markupnw(text, len, markup) <= w)
			break;
		
		while(len > 1 && !utf8isfirstbyte(text[len - 1]))
			len--;
		len--;
	}
	if(!len)
		return;
	memcpy(buf, text, len);
	if(len < olen)
		for(i = len; i && i > len - 3; buf[--i] = '.');
	if(markup)
		pango_layout_set_markup(dc.font.layout, buf, len);
 	else
		pango_layout_set_text(dc.font.layout, buf, len);
	
	d=XftDrawCreate(dpy, window, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));
	pango_xft_render_layout(d,
		(col == dc.norm ? dc.xft.norm : dc.xft.sel) + ColFG,
		dc.font.layout, x * PANGO_SCALE, y * PANGO_SCALE);
	if(markup) /* clear markup attributes */
		pango_layout_set_attributes(dc.font.layout, NULL);
	XftDrawDestroy(d);
}