#include <dbus/dbus.h>
#include <alsa/asoundlib.h>
#include "dwm.h"

#define MENU_WIDTH 128

static const char *mpris="org.mpris.MediaPlayer2.";
static struct {
	DBusConnection* connection;
	DBusError error;
	dbus_uint32_t serial;
	const char *interface;
	const char *object;
} dbus={
	NULL,
	{0},
	0,
	"org.freedesktop.DBus",
	"/org/freedesktop/DBus",
};

static struct {
	snd_mixer_t *handle;
	snd_mixer_elem_t* elem;
	snd_mixer_selem_id_t *sid;
	const char* card;
	const char* mix_name;
	int mix_index;
	long minv;
	long maxv;
} alsa={
	NULL,
	NULL,
	NULL,
	"default",
	"Master",
	0,
};

static struct {
	Window window;
	GC gc;
	int x, y, w, h;
} menu;

static struct MEDIAPLAYER {
	const char *id;
	const char *name;
	struct MEDIAPLAYER *next;
} *mediaplayer=NULL;

static void mediaplayer_register(const char *id) {
	struct MEDIAPLAYER **mp;
	for(mp=&mediaplayer; *mp; mp=&((*mp)->next)) {
		if(!strcmp(id, (*mp)->id))
			return;
	}
	*mp=malloc(sizeof(struct MEDIAPLAYER));
	(*mp)->id=malloc(strlen(id)+1);
	(*mp)->name=NULL;
	(*mp)->next=NULL;
	strcpy((void *) (*mp)->id, id);
}

static void mediaplayer_deregister(const char *id) {
	struct MEDIAPLAYER **mp, *mp_next;
	for(mp=&mediaplayer; *mp; mp=&((*mp)->next)) {
		if(!strcmp(id, (*mp)->id)) {
			mp_next=(*mp)->next;
			free((void *) (*mp)->id);
			free((void *) (*mp)->name);
			free(*mp);
			*mp=mp_next;
			return;
		}
	}
}

static void mediaplayer_deregister_all() {
	struct MEDIAPLAYER *mp_next;
	while(mediaplayer) {
		mp_next=mediaplayer->next;
		free((void *) mediaplayer->id);
		free((void *) mediaplayer->name);
		free(mediaplayer);
		mediaplayer=mp_next;
	}
}

static void menu_open(Indicator *indicator) {
	int i;
	struct MEDIAPLAYER *mp;
	/*XSetWindowAttributes wa={
		.override_redirect=True,
		//.background_pixel=XBlackPixel(dpy, screen),
		.event_mask=ButtonPressMask|ExposureMask,
	};*/
	for(mp=mediaplayer, i=0; mp; mp=mp->next, i++);
	/*menu=XCreateWindow(dpy, root,
		0, bh, MENU_WIDTH, bh*2*i, 0, DefaultDepth(dpy, screen),
		InputOutput, DefaultVisual(dpy, screen),
		CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa
	);*/
	menu.x=selmon->mx+indicator->x-MENU_WIDTH+indicator->width;
	menu.y=bh;
	menu.w=MENU_WIDTH;
	menu.h=bh*2*(i+1);
	menu.window=XCreateSimpleWindow(dpy, root, 
		menu.x, menu.y, menu.w, menu.h,
		1, dc.sel[ColBorderFloat].pixel, dc.norm[ColBG].pixel
	);
	menu.gc=XCreateGC(dpy, menu.window, 0, 0);
	XSelectInput(dpy, menu.window, ExposureMask);
	XDefineCursor(dpy, menu.window, cursor[CurNormal]);
	//indicator_music_expose(indicator, menu);
	XMapRaised(dpy, menu.window);
}
static void menu_close() {
	XFreeGC(dpy, menu.gc);
	XUnmapWindow(dpy, menu.window);
	XDestroyWindow(dpy, menu.window);
}

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

static void check_bus() {
	DBusMessage* msg;
	DBusMessageIter args;
	char *player, *oldowner, *newowner;
	
	while(1) {
		dbus_connection_read_write(dbus.connection, 0);

		if(!(msg=dbus_connection_pop_message(dbus.connection)))
			break;
		
		if(dbus_message_is_signal(msg, dbus.interface, "NameOwnerChanged")) {
			if(!(dbus_message_iter_init(msg, &args)&&dbus_message_iter_get_arg_type(&args)==DBUS_TYPE_STRING))
				continue;
			
			dbus_message_iter_get_basic(&args, &player);
			dbus_message_iter_next(&args);
			dbus_message_iter_get_basic(&args, &oldowner);
			dbus_message_iter_next(&args);
			dbus_message_iter_get_basic(&args, &newowner);
			if(!strncmp(player, mpris, strlen(mpris))) {
				if(!strlen(oldowner)&&strlen(newowner)) {
					printf("Registered mediaplayer %s\nSending command to start playing music\n", player+strlen(mpris));
					mediaplayer_register(player+strlen(mpris));
				}
				if(!strlen(newowner)) {
					printf("Deregistered mediaplayer %s (program exited)\n", player+strlen(mpris));
					mediaplayer_deregister(player+strlen(mpris));
				}
			}
		}

		dbus_message_unref(msg);
	}
}

static int volume_get() {
	long volume;
	if(snd_mixer_selem_get_playback_volume(alsa.elem, 0, &volume)<0)
		return -1;
	
	volume-=alsa.minv;
	return (100*volume)/(alsa.maxv-alsa.minv);
}

static void volume_set(int volume) {
	if(volume<0)
		volume=0;
	if(volume>100)
		volume=100;
	
	volume=volume*(alsa.maxv-alsa.minv)/100+alsa.minv;
	snd_mixer_selem_set_playback_volume(alsa.elem, 0, volume);
	snd_mixer_selem_set_playback_volume(alsa.elem, 1, volume);
}

int indicator_music_init(Indicator *indicator) {
	DBusMessage* msg;
	DBusMessageIter args, element;
	DBusPendingCall* pending;
	char *player;
	int current_type;
	
	/*Init dbus*/
	dbus_error_init(&dbus.error);
	dbus.connection = dbus_bus_get(DBUS_BUS_SESSION, &dbus.error);
	if(dbus_error_is_set(&dbus.error)) { 
		dbus_error_free(&dbus.error);
		return -1;
	}
	if(!dbus.connection) { 
		return -1;
	}
	
	if(!(msg=dbus_message_new_method_call(dbus.interface, dbus.object, dbus.interface, "ListNames")))
		return -1;
	
	if(!dbus_connection_send_with_reply(dbus.connection, msg, &pending, -1)) {
		dbus_connection_unref(dbus.connection);
		return -1;
	}
	if(!pending) {
		dbus_message_unref(msg);
		dbus_connection_unref(dbus.connection);
		return -1;
	}
	dbus_connection_flush(dbus.connection);
	dbus_message_unref(msg);
	dbus_pending_call_block(pending);

	if (!(msg=dbus_pending_call_steal_reply(pending))) {
		dbus_connection_unref(dbus.connection);
		return -1;
	}
	dbus_pending_call_unref(pending);

	if(!(dbus_message_iter_init(msg, &args)&&dbus_message_iter_get_arg_type(&args)==DBUS_TYPE_ARRAY)) {
		dbus_message_unref(msg);
		dbus_connection_unref(dbus.connection);
		return -1;
	}
	
	dbus_message_iter_recurse (&args, &element);
	printf("Found media players:\n");
	while((current_type=dbus_message_iter_get_arg_type(&element))!=DBUS_TYPE_INVALID) {
		if(current_type!=DBUS_TYPE_STRING)
			continue;
		dbus_message_iter_get_basic(&element, &player);
		if(!strncmp(player, mpris, strlen(mpris))) {
			printf(" * %s\n", player+strlen(mpris));
			mediaplayer_register(player+strlen(mpris));
		}
		dbus_message_iter_next(&element);
	}
	dbus_message_unref(msg);
	
	dbus_bus_add_match(dbus.connection, "type='signal',interface='org.freedesktop.DBus',member='NameOwnerChanged'", &dbus.error);
	dbus_connection_flush(dbus.connection);
	if(dbus_error_is_set(&dbus.error)) { 
		dbus_connection_unref(dbus.connection);
		mediaplayer_deregister_all();
		return -1;
	}
	
	/*init alsa*/
	snd_mixer_selem_id_alloca(&alsa.sid);
	snd_mixer_selem_id_set_index(alsa.sid, alsa.mix_index);
	snd_mixer_selem_id_set_name(alsa.sid, alsa.mix_name);
	
	if((snd_mixer_open(&alsa.handle, 0))<0)
		return -1;
	if((snd_mixer_attach(alsa.handle, alsa.card))<0) {
		snd_mixer_close(alsa.handle);
		return -1;
	}
	if((snd_mixer_selem_register(alsa.handle, NULL, NULL))<0) {
		snd_mixer_close(alsa.handle);
		return -1;
	}
	if(snd_mixer_load(alsa.handle)<0) {
		snd_mixer_close(alsa.handle);
		return -1;
	}
	if(!(alsa.elem=snd_mixer_find_selem(alsa.handle, alsa.sid))) {
		snd_mixer_close(alsa.handle);
		return -1;
	}
	
	snd_mixer_selem_get_playback_volume_range (alsa.elem, &alsa.minv, &alsa.maxv);
	fprintf(stderr, "Volume range <%li,%li>\n", alsa.minv, alsa.maxv);
	
	return 0;
}

void indicator_music_update(Indicator *indicator) {
	check_bus();
	snd_mixer_handle_events(alsa.handle);
	sprintf(indicator->text, " ♫ %i%% ", volume_get());
}

void indicator_music_expose(Indicator *indicator, Window window) {
	int y=0;
	struct MEDIAPLAYER *mp;
	if(window!=menu.window)
		return;
	printf("expose\n");
	
	XSetForeground(dpy, menu.gc, dc.norm[ColFG].pixel);
	//XFillRectangle(dpy, window, menu.gc, 10, 10, 20, 20);
	draw_text(0, 0, menu.w, bh, dc.norm, "Mute");
	draw_text(0, bh, menu.w, bh, dc.norm, "volume slider");
	y=2*bh;
	for(mp=mediaplayer; mp; mp=mp->next, y+=bh*2)
		draw_text(0, y, menu.w, bh, dc.norm, mp->id);
	XFlush(dpy);
}

void indicator_music_mouse(Indicator *indicator, unsigned int button) {
	switch(button) {
		case Button1:
		case Button3:
			printf("clicked music indicator\n");
			if((indicator->active=!indicator->active))
				menu_open(indicator);
			else
				menu_close();
			break;
		case Button4:
			snd_mixer_handle_events(alsa.handle);
			volume_set(volume_get()+10);
			break;
		case Button5:
			snd_mixer_handle_events(alsa.handle);
			volume_set(volume_get()-10);
			break;
	}
}
