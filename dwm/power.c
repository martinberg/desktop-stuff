#include <stdlib.h>
#include "dwm.h"
#include "dbus.h"

static void send(const char *destination, const char *interface, const char *object, const char *method) {
	DBusMessage *msg = NULL, *reply = NULL;
	DBusError error;
	
	msg=dbus_message_new_method_call(destination, object, interface, method);
	if(!msg)
		return;
	
	//dbus_connection_send(dbus.system.connection, msg, &dbus.system.serial);
	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(dbus.system.connection, msg, -1, &error);
	//dbus_connection_flush(dbus.system.connection);
	
	if(!reply)
		goto fail;
	
	dbus_error_init(&error);
	dbus_message_get_args(reply, &error, DBUS_TYPE_INVALID);
	
	fail:
	dbus_message_unref(msg);
	dbus_message_unref(reply);
	//dbus.system.serial++;
}

void power_logout() {
	quit(NULL);
}

void power_shutdown() {
	send("org.freedesktop.ConsoleKit", "org.freedesktop.ConsoleKit.Manager", "/org/freedesktop/ConsoleKit/Manager", "Stop");
}

void power_restart() {
	send("org.freedesktop.ConsoleKit", "org.freedesktop.ConsoleKit.Manager", "/org/freedesktop/ConsoleKit/Manager", "Restart");
}

void power_suspend() {
	send("org.freedesktop.UPower", "org.freedesktop.UPower", "/org/freedesktop/UPower", "Suspend");
}

void power_hibernate() {
	send("org.freedesktop.UPower", "org.freedesktop.UPower", "/org/freedesktop/UPower", "Hibernate");
}
