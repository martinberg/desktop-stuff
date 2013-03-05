CFLAGS	+=	`pkg-config --cflags dbus-1` -g -O0
LDFLAGS	+=	`pkg-config --libs dbus-1`

all:
	gcc -o dbus-mpris dbus-mpris.c $(CFLAGS) $(LDFLAGS)

