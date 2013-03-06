#include <dbus/dbus.h>
#include "dwm.h"

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

int indicator_music_init(Indicator *indicator) {
	DBusMessage* msg;
	DBusMessageIter args, element;
	DBusPendingCall* pending;
	char *player;
	int current_type;
	
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
	
	return 0;
}

void indicator_music_update(Indicator *indicator) {
	sprintf(indicator->text, " ♫ %s ", "100%");
}

void indicator_music_mouse(Indicator *indicator, unsigned int button) {
	printf("clicked music indicator\n");
}
