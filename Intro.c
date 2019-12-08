#include <windows.h>
#include <gl\gl.h>

#include "common.h"
#include "graphics.h"
#include "texture.h"
#include "intro.h"
#include "text.h"

#define CENTRE_X (VIEW_SIZE_X >> 1)
#define CENTRE_Y (VIEW_SIZE_Y >> 1)

void intro_process(unsigned long time)
{
	glClear(GL_COLOR_BUFFER_BIT);
	graphics_orthographic();

	glShadeModel(GL_FLAT);
	glEnable(GL_TEXTURE_2D);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBindTexture(GL_TEXTURE_2D, texture_id(1));

	glBegin(GL_QUADS);
	glTexCoord2d(0, 0);
	glVertex2f(CENTRE_X - 256, CENTRE_Y - 64);

	glTexCoord2d(1, 0);
	glVertex2f(CENTRE_X + 256, CENTRE_Y - 64);

	glTexCoord2d(1, 1);
	glVertex2f(CENTRE_X + 256, CENTRE_Y + 64);

	glTexCoord2d(0, 1);
	glVertex2f(CENTRE_X - 256, CENTRE_Y + 64);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void intro_end(void)
{
	texture_free(1);
}

void intro_setup(void)
{
	graphics_setup();

	// Load the printing font.
	if(!text_font("data\\font.sff", 0))
		exit(1);

	// Load the splash image.
	if(!texture_load("data\\intro.bmp", 1))
		exit(1);
}