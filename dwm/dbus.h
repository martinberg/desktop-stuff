#ifndef __DBUS_H_
#define __DBUS_H_
#include <dbus/dbus.h>

struct DBus {
	struct {
		DBusConnection* connection;
		DBusError error;
		dbus_uint32_t serial;
	} session;
	struct {
		DBusConnection* connection;
		DBusError error;
		dbus_uint32_t serial;
	} system;
	const char *interface;
	const char *interface_prop;
	const char *object;
};

extern struct DBus dbus;

int dbus_init();
int keyring_init();
int ck_init();
int ck_exit();

#endif