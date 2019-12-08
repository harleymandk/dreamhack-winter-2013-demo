#include <string.h>
#include <stdlib.h>

#include "text.h"
#include "fps.h"

static char framesps[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static char f[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static unsigned long last_time = 0;
static unsigned long frames = 0;

void fps_update(unsigned long time_delta)
{
	memset(f, 0, 8);
	itoa(time_delta, f, 10);
	text_print(320, 30, f, 1.0F);

	if(time_delta - last_time >= 1000) {
		// A second has passed.
		memset(framesps, 0, 8);
		itoa(frames, framesps, 10);

		last_time = time_delta;
		frames = 0;
	}

	text_print(320, 0, framesps, 1.0F);
	frames++;
}