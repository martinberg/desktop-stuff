#include <dbus/dbus.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *mpris="org.mpris.MediaPlayer2.";
DBusConnection* conn;
DBusError err;
dbus_uint32_t serial=0;

void query() {
	DBusMessage* msg;
	DBusMessageIter args;
	DBusPendingCall* pending;
	int ret;
	char *player;
	dbus_uint32_t level;
	int current_type;
	DBusMessageIter subiter;

	// create a new method call and check for errors
	if (!(msg=dbus_message_new_method_call(
		"org.freedesktop.DBus", // target for the method call
		"/org/freedesktop/DBus", // object to call on
		"org.freedesktop.DBus", // interface to call on
		"ListNames" // method name
	))) { 
		fprintf(stderr, "Message Null\n");
		return;
	}
	
	// send message and get a handle for a reply
	if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) {
		// -1 is default timeout
		fprintf(stderr, "Out Of Memory!\n"); 
		exit(1);
	}
	if (!pending) { 
		fprintf(stderr, "Pending Call Null\n"); 
		return;
	}
	dbus_connection_flush(conn);
	dbus_message_unref(msg);
	
	// block until we recieve a reply
	dbus_pending_call_block(pending);

	// get the reply message
	msg = dbus_pending_call_steal_reply(pending);
	if (NULL == msg) {
		fprintf(stderr, "Reply Null\n"); 
		exit(1); 
	}
	dbus_pending_call_unref(pending);

	// read the parameters
	if (!dbus_message_iter_init(msg, &args))
		fprintf(stderr, "Message has no arguments!\n"); 
	if(dbus_message_iter_get_arg_type(&args)!=DBUS_TYPE_ARRAY) {
		fprintf(stderr, "Message has wrong argument type!\n"); 
		return;
	}
	
	dbus_message_iter_recurse (&args, &subiter);
	printf("Found media players:\n");
	while((current_type = dbus_message_iter_get_arg_type (&subiter)) != DBUS_TYPE_INVALID) {
		if(current_type!=DBUS_TYPE_STRING)
			continue;
		dbus_message_iter_get_basic(&subiter, &player);
		if(!strncmp(player, mpris, strlen(mpris))) {
			printf(" * %s\n", player+strlen(mpris));
		}
		dbus_message_iter_next(&subiter);
	}
	
	dbus_message_unref(msg);
}

/**
 * Listens for signals on the bus
 */
void receive() {
	DBusMessage* msg;
	DBusMessageIter args;
	int ret;
	char *player, *oldowner, *newowner;
	
	// add a rule for which messages we want to see
	dbus_bus_add_match(conn, "type='signal',interface='org.freedesktop.DBus',member='NameOwnerChanged'", &err);
	dbus_connection_flush(conn);
	if(dbus_error_is_set(&err)) { 
		fprintf(stderr, "Match Error (%s)\n", err.message);
		exit(1); 
	}

	//listening for signals being emmitted
	while(1) {

		// non blocking read of the next available message
		dbus_connection_read_write(conn, 0);

		// loop again if we haven't read a message
		if (!(msg=dbus_connection_pop_message(conn))) { 
			sleep(1);
			continue;
		}

		// check if the message is a signal from the correct interface and with the correct name
		if(dbus_message_is_signal(msg, "org.freedesktop.DBus", "NameOwnerChanged")) {
			// read the parameters
			if(!dbus_message_iter_init(msg, &args))
				fprintf(stderr, "Message Has No Parameters\n");
			else if(DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) 
				fprintf(stderr, "Argument is not string!\n");
			
			dbus_message_iter_get_basic(&args, &player);
			dbus_message_iter_next(&args);
			dbus_message_iter_get_basic(&args, &oldowner);
			dbus_message_iter_next(&args);
			dbus_message_iter_get_basic(&args, &newowner);
			if(!strncmp(player, mpris, strlen(mpris))) {
				if(!strlen(oldowner)&&strlen(newowner)) {
					printf("Registered mediaplayer %s\nSending command to start playing music\n", player+strlen(mpris));
					DBusMessage *msg2 = dbus_message_new_method_call(
						player,
						"/org/mpris/MediaPlayer2",
						"org.mpris.MediaPlayer2.Player",
						"Play"
					);
					dbus_connection_send(conn, msg2, &serial);
					dbus_connection_flush(conn);
					dbus_message_unref(msg2);
					serial++;
				}
				if(!strlen(newowner))
					printf("Deregistered mediaplayer %s (program exited)\n", player+strlen(mpris));
			}
		}

		dbus_message_unref(msg);
	}
}

int main(int argc, char** argv) {
	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if(dbus_error_is_set(&err)) { 
		fprintf(stderr, "Connection Error (%s)\n", err.message); 
		dbus_error_free(&err);
	}
	if(!conn) { 
		exit(1); 
	}
	query();
	receive();
	
	//dbus_connection_close(conn);
	return 0;
}
