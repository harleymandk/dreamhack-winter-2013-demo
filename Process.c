/*
Aside from windows message processing, this module is
the highest level of abstraction. The function
process_control(...) is called every frame. It's
responsible for handing control to the active scene and
telling that scene how long it's been running. It
manages fading between scenes and framerate profiling.
*/

#include <windows.h>

#include "graphics.h"
#include "common.h"
#include "process.h"
#include "credits.h"
#include "rohan.h"
#include "demo.h"
//#include "moria.h" Wasn't finished in time.
#include "intro.h"
#include "fps.h"

#define FADE_DURATION 2000
#define NUM_SCENES 4

/*
All scenes are time based. Everything is updated
and drawn given the amount of time that has passed
since that scene started.
*/

typedef struct
{
	void (*update) (unsigned long);	// Update function. Draw objects based on current time.
	void (*setup) (void);			// Startup function. Allocated data etc.
	void (*end) (void);				// Shut down function. Free data etc.
	unsigned long duration;
} scene_t;

static scene_t scene_index[NUM_SCENES] = {
	{	// The opening LOTR graphic scene.
		intro_process,
		intro_setup,
		intro_end,
		7000
	},

	{	// The spinning rings scene.
		ring_poem_process,
		ring_poem_setup,
		ring_poem_end,
		58000
	},

	{	// The hilly terrain scene.
		rohan_terrain_process,
		rohan_terrain_setup,
		rohan_terrain_end,
		35000
	},

//	{	// Wasn't finished in time. Moria tunnels.
//		moria_process,
//		moria_setup,
//		moria_end,
//		35000
//	},

	{	// A brief credits list.
		credits_process,
		credits_setup,
		credits_end,
		67000
	}
};

static unsigned long process_timer;
static int scene_running = 0;
static int scene = 0;

static void process_timer_reset(void)
{
	process_timer = timeGetTime();
}

static unsigned long process_timer_get(void)
{
	return (unsigned long) (timeGetTime() - process_timer);
}

int process_control(void)
{
	unsigned long elapsed;

	if(!scene_running) {
		// A scene needs to be initialized.
		scene_index[scene].setup();
		process_timer_reset();
		scene_running = 1;
	}

	elapsed = process_timer_get();
	scene_index[scene].update(elapsed);

	// Fade between the scenes.
	if(elapsed >= scene_index[scene].duration - FADE_DURATION)
		graphics_fade(1.0F / FADE_DURATION * (elapsed
			- (scene_index[scene].duration - FADE_DURATION)));
	else if(elapsed < FADE_DURATION)
		graphics_fade(1.0F / FADE_DURATION * (FADE_DURATION - elapsed));

#ifdef BUILD_DEBUG
	fps_update(elapsed);
#endif

	if(process_timer_get() >= scene_index[scene].duration) {
		// This scene has ended. Move on to the next one.
		scene_index[scene].end();
		scene_running = 0;
		scene++;
	}

	// Exit when all scenes are finished.
	if(scene == NUM_SCENES || GetAsyncKeyState(VK_ESCAPE)) {
		if(scene_running) {
			scene_index[scene].end();

			if(scene != 3)
				scene_index[3].end();
		}

		return 1;
	}

	return 0;
}