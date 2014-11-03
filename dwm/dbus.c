struct {
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
} dbus={
	NULL,
	{0},
	{},
	{},
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
	/*if(!dbus.session.connection) { 
		return -1;
	}*/
	
	/*Init system bus*/
	dbus_error_init(&dbus.system.error);
	dbus.connection = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus.system.error);
	if(dbus_error_is_set(&dbus.system.error)) { 
		dbus_error_free(&dbus.system.error);
		dbus.system.connection = NULL;
	}
	/*if(!dbus.system.connection) { 
		return -2;
	}*/
	
	return 0;
}