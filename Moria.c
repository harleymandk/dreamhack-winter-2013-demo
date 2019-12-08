/*
This scene wasn't finished in time for the deadline.
Fortunately the very modular structure of the demo
framework has allowed me to just drop the scene out
with no repercussions. I wanted to show Gandalf and
the Balrog falling in the tunnel, but never got round
to it. The tunnel is generated form a heightmap,
which tiles. The code reads from a window into the
heightmap and scrolls the tunnel data along.
*/

/*
#include <windows.h>
#include <gl\gl.h>
#include <math.h>
#include <stdio.h>

#include "graphics.h"
#include "texture.h"
#include "moria.h"
#include "text.h"

#define TUNNEL_SCALE_RAD 150
#define TUNNEL_VERTICES_X 64
#define TUNNEL_VERTICES_R 65
#define TUNNEL_VERTICES_Z 32
#define TUNNEL_SCALE_Z 10
#define HM_Y 256
#define HM_X 64

static float vertex[TUNNEL_VERTICES_R * TUNNEL_VERTICES_Z * 3];
static float colour[TUNNEL_VERTICES_R * TUNNEL_VERTICES_Z * 3];
static float texture[TUNNEL_VERTICES_R * TUNNEL_VERTICES_Z * 3];
static unsigned char height[HM_X * HM_Y];

static float shift;
static int depth;

static void scroll_tunnel(void)
{
	float *a, *b;
	int i;

	b = &vertex[TUNNEL_VERTICES_R * 3];
	a = vertex;

	for(i = 0; i < TUNNEL_VERTICES_R * (TUNNEL_VERTICES_Z - 1) * 3; i += 3) {
		*a++ = *b++;
		*a++ = *b++;
		a++; // Depth (z) stays fixed.
		b++; // Depth (z) stays fixed.
	}
}

static void get_radii_from_scanline(unsigned char *l, float *v)
{
	float c, s;
	int i;

	for(i = 0; i < TUNNEL_VERTICES_X; i++) {
		// Start with a circle.
		c = (float) cos((6.282 / TUNNEL_VERTICES_X) * i) * (TUNNEL_SCALE_RAD - l[i] * 0.5F);
		s = (float) sin((6.282 / TUNNEL_VERTICES_X) * i) * (TUNNEL_SCALE_RAD - l[i] * 0.5F);

		// Factor in the terrain height.
		v[i * 3 + 0] = c;
		v[i * 3 + 1] = s;
	}

	// Copy first vertex into last vertex.
	v[i * 3 + 0] = v[0];
	v[i * 3 + 1] = v[1];
}

void moria_process(unsigned long time)
{
	int i, z;

	glClear(GL_COLOR_BUFFER_BIT);
	graphics_perspective();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glShadeModel(GL_SMOOTH);

	// Enable texturing.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_id(1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Time based movement.
	shift = (time & 1023) * 0.01F;

	if(shift >= TUNNEL_SCALE_Z) {
		// Shift existing vertices up one column.
		scroll_tunnel();
		depth++;
		depth &= 255;
		shift = 0;

		get_radii_from_scanline(&height[depth * HM_X],
			&vertex[TUNNEL_VERTICES_R * (TUNNEL_VERTICES_Z - 1) * 3]);
	}

	glRotatef(180, 0, 1, 0);
	glTranslatef(0, 0, 50 - shift);

	// Tunnel rings draw farthest-to-nearest. Avoids depth buffer tests.
	for(z = TUNNEL_VERTICES_Z - 2; z >= 0; z--) {
		glBegin(GL_TRIANGLE_STRIP);

		for(i = 0; i < TUNNEL_VERTICES_R; i++) {
			glTexCoord2fv(&texture[((z + 0) * TUNNEL_VERTICES_R + i) * 2]);
			glColor3fv(&colour[((z + 0) * TUNNEL_VERTICES_R + i) * 3]);
			glVertex3fv(&vertex[((z + 0) * TUNNEL_VERTICES_R + i) * 3]);

			glTexCoord2fv(&texture[((z + 1) * TUNNEL_VERTICES_R + i) * 2]);
			glColor3fv(&colour[((z + 1) * TUNNEL_VERTICES_R + i) * 3]);
			glVertex3fv(&vertex[((z + 1) * TUNNEL_VERTICES_R + i) * 3]);
		}

		glEnd();
	}

	glDisable(GL_CULL_FACE);
	graphics_orthographic();
	glDisable(GL_TEXTURE_2D);
}

void moria_end(void)
{
	texture_free(1);
}

static int read_height_data(char *pathname)
{
	FILE *fp;

	if(!(fp = fopen(pathname, "rb")))
		return 0;

	fread(height, HM_Y * HM_X, sizeof(unsigned char), fp);
	fclose(fp);

	return 1;
}

void moria_setup(void)
{
	int j;

	// Modification to depth here carries over to moria_process.
	depth = 0;
	shift = 0;

	// Must be 64 x 256 8bpp image.
	if(!read_height_data("data\\tunnel1.raw"))
		exit(1);

	// Any image will suffice here.
	if(!texture_load("data\\tunnelrock1.bmp", 1))
		exit(1);

	// Generate data for the tunnel. Uses a window into the heightmap.
	for(depth = 0; depth < TUNNEL_VERTICES_Z; depth++) {
		get_radii_from_scanline(&height[(depth & 255) * HM_X],
			&vertex[TUNNEL_VERTICES_R * depth * 3]);

		// Set depth for this ring. Constant.
		for(j = 0; j < TUNNEL_VERTICES_R; j++) {
			// Vertex depth.
			vertex[(depth * TUNNEL_VERTICES_R + j) * 3 + 2] = (float) depth * TUNNEL_SCALE_Z;

			// Depth related colour.
			colour[(depth * TUNNEL_VERTICES_R + j) * 3 + 0] = 1.0F - depth * (1.0F / TUNNEL_VERTICES_Z);
			colour[(depth * TUNNEL_VERTICES_R + j) * 3 + 1] = 1.0F - depth * (1.0F / TUNNEL_VERTICES_Z);
			colour[(depth * TUNNEL_VERTICES_R + j) * 3 + 2] = 1.0F - depth * (1.0F / TUNNEL_VERTICES_Z);

			// Angle and depth related texture coordinates.
			texture[(depth * TUNNEL_VERTICES_R + j) * 2 + 0] = (float) j;
			texture[(depth * TUNNEL_VERTICES_R + j) * 2 + 1] = (float) depth;
		}
	}
}

*/