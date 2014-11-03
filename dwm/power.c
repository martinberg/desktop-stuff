static void send() {
	DBusMessage *msg;
	char player[256];
	sprintf(player, "%s%s", mpris.base, id);
	
	msg=dbus_message_new_method_call(player, mpris.object, mpris.interface_player, actions[action]);
	dbus_connection_send(dbus.system.connection, msg, &dbus.system.serial);
	dbus_connection_flush(dbus.system.connection);
	dbus_message_unref(msg);
	dbus.system.serial++;
}

power_shutdown() {
	//dbus-send --system --print-reply --dest="org.freedesktop.ConsoleKit" /org/freedesktop/ConsoleKit/Manager org.freedesktop.ConsoleKit.Manager.Stop
}

power_reboot() {
	//dbus-send --system --print-reply --dest="org.freedesktop.ConsoleKit" /org/freedesktop/ConsoleKit/Manager org.freedesktop.ConsoleKit.Manager.Restart
}

power_suspend() {
	//dbus-send --system --print-reply --dest="org.freedesktop.UPower" /org/freedesktop/UPower org.freedesktop.UPower.Suspend
}

power_hibernate() {
	//dbus-send --system --print-reply --dest="org.freedesktop.UPower" /org/freedesktop/UPower org.freedesktop.UPower.Hibernate
}
