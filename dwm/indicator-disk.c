#include <sys/statvfs.h>

#include "dwm.h"

int indicator_disk_init(Indicator *indicator) {
	return 0;
}

void indicator_disk_update(Indicator *indicator) {
	struct statvfs sbuf;
	
	if(statvfs("/", &sbuf)>=0)
		sprintf(indicator->text, " ⛁ %lu %% ", 100*(sbuf.f_blocks-sbuf.f_bavail)/sbuf.f_blocks);
}

void indicator_disk_expose(Indicator *indicator, Window window) {
}

Bool indicator_disk_haswindow(Indicator *indicator, Window window) {
	return False;
}

void indicator_disk_mouse(Indicator *indicator, XButtonPressedEvent *ev) {
	/*pop up disk info*/
	/*if(ev->button==Button1||ev->button==Button3) {
		printf("clicked time indicator\n");
		indicator->active=!indicator->active;
	}*/
}
