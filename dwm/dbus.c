#include <stdio.h>
#include <stdlib.h>
#include "dbus.h"

struct DBus dbus={
	{
		NULL,
		{0},
		0
	}, {
		NULL,
		{0},
		0
	},
	"org.freedesktop.DBus",
	"org.freedesktop.DBus.Properties",
	"/org/freedesktop/DBus",
};

int dbus_init() {
	/*Init session bus*/
	dbus_error_init(&dbus.session.error);
	dbus.session.connection = dbus_bus_get(DBUS_BUS_SESSION, &dbus.session.error);
	if(dbus_error_is_set(&dbus.session.error)) { 
		dbus_error_free(&dbus.session.error);
		dbus.session.connection = NULL;
	}
	if(!dbus.session.connection) { 
		return -1;
	}
	
	/*Init system bus*/
	dbus_error_init(&dbus.system.error);
	dbus.system.connection = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus.system.error);
	if(dbus_error_is_set(&dbus.system.error)) { 
		dbus_error_free(&dbus.system.error);
		dbus.system.connection = NULL;
	}
	if(!dbus.system.connection) { 
		return -1;
	}
	
	return 0;
}

int ck_init() {
	DBusMessage *msg, *reply;
	DBusError error;
	char *cookie;
	
	msg=dbus_message_new_method_call("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", "OpenSession");
	if(!msg) {
		return -1;
	}
	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(dbus.system.connection, msg, -1, &error);
	
	if(!reply) {
		dbus_message_unref(msg);
		return -1;
	}
	
	dbus_error_init(&error);
	if(!dbus_message_get_args(reply, &error, DBUS_TYPE_STRING, &cookie, DBUS_TYPE_INVALID)) {
		dbus_message_unref(msg);
		dbus_message_unref(reply);
		return -1;
	}
	
	
	setenv("XDG_SESSION_COOKIE", cookie, 1);
	dbus_message_unref(msg);
	dbus_message_unref(reply);
	return 0;
}

int keyring_init() {
	return 0;
	FILE *f;
	char *buf;
	
	if(!(f = popen("gnome-keyring-daemon --start --components=pkcs11,secrets,ssh", "r")))
		return -1;
	
	while(!feof(f)) {
		buf = NULL;
		fscanf(f, "%m[^\n]\n", &buf);
		if(buf)
			putenv(buf);
	}
	
	pclose(f);
	return 0;
}

int ck_exit() {
	int result;
	DBusMessage *msg, *reply;
	DBusError error;
	char *cookie = getenv("XDG_SESSION_COOKIE");
	
	msg=dbus_message_new_method_call("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", "CloseSession");
	if(!msg)
		return -1;
	
	if(!dbus_message_append_args(msg, DBUS_TYPE_STRING, &cookie, DBUS_TYPE_INVALID)) {
		dbus_message_unref(msg);
		return -1;
	}
	
	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(dbus.system.connection, msg, -1, &error);
	
	if(!reply) {
		dbus_message_unref(msg);
		return -1;
	}
	
	dbus_error_init(&error);
	if(!dbus_message_get_args(reply, &error, DBUS_TYPE_BOOLEAN, &result, DBUS_TYPE_INVALID)) {
		dbus_message_unref(msg);
		dbus_message_unref(reply);
		return -1;
	}
	
	dbus_message_unref(msg);
	dbus_message_unref(reply);
	
	return 0;
}
