#include <windows.h>
#include <gl\glu.h>
#include <gl\gl.h>

#include "common.h"
#include "graphics.h"

void graphics_setup(void)
{
	// Set the view port dimensions.
	glViewport(0, 0, VIEW_SIZE_X, VIEW_SIZE_Y);

	// Reset the projection matrix.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Reset the model view matrix.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Reset texture matrix.
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	// Frame and depth buffer clear values.
	glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
	glClearDepth(1.0F);
}

void graphics_orthographic(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0F, VIEW_SIZE_X, VIEW_SIZE_Y, 0.0F, 0.0F, 1.0F);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void graphics_perspective(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0F, (float) VIEW_SIZE_X / (float) VIEW_SIZE_Y, 1.5F, 2000.0F);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*
Fade by drawing a variably transparent black
rectangle across the screen. The higher the
opacity, the closer to pure black the screen
will be faded. Note well the blending modes.
*/

void graphics_fade(float opacity)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(0, 0, 0, opacity);

	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(VIEW_SIZE_X, 0);
	glVertex2i(VIEW_SIZE_X, VIEW_SIZE_Y);
	glVertex2i(0, VIEW_SIZE_Y);
	glEnd();

	glDisable(GL_BLEND);
}

void cross_product(float *r, const float *a, const float *b)
{
	r[0] = a[1] * b[2] - b[1] * a[2];
	r[1] = a[2] * b[0] - b[2] * a[0];
	r[2] = a[0] * b[1] - b[0] * a[1];
}

void vector_subtract(float *r, const float *a, const float *b)
{
	r[0] = a[0] - b[0];
	r[1] = a[1] - b[1];
	r[2] = a[2] - b[2];
}