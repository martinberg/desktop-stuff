#include "dwm.h"

void indicator_music_update(Indicator *indicator) {
	sprintf(indicator->text, " ♫ %s ", "100%");
}

void indicator_music_mouse(Indicator *indicator, unsigned int button) {
	printf("clicked music indicator\n");
}
