#include <windows.h>
#include <gl\glu.h>
#include <gl\glext.h>	// GL 1.2 texture clamping.
#include <gl\gl.h>
#include <math.h>
#include <stdio.h>

#include "common.h"
#include "graphics.h"
#include "texture.h"
#include "rohan.h"
#include "text.h"

#define TERRAIN_SCALE_Y 0.5F
#define TERRAIN_SCALE_X 5
#define TERRAIN_SCALE_Z 5
#define TERRAIN_X 64
#define TERRAIN_Y 64

#define MAX_POINTS 6 * 3 + 1
#define BEZIER_LOD 10
#define NUM_MODELS 45

#pragma pack(1)

typedef struct {
	float x;
	float y;
	float z;
} vertex_t;

typedef struct {
	unsigned short vertex[3];
	float u[3];
	float v[3];
} triangle_t;

// Models were converted from .ms3d. My format is simple.
typedef struct {
	vertex_t *vertex;
	triangle_t *triangle;
	unsigned short num_vertices;
	unsigned short num_triangles;
} model_t;

#pragma pack()

typedef struct {
	float shade;
	float yaw;		// Heading for the horse.
	float x;		// 0 to 320
	float y;
	float z;		// 0 to 320
	model_t **frame;
} horse_t;

typedef struct {
	float pitch;
	float yaw;
	float roll;
	float x;
	float y;
	float z;
	float delta;
	int point;
	unsigned long last;
} camera_t;

typedef struct {
	float x;
	float y;
} point_t;

static unsigned int indices[(TERRAIN_Y - 1) * TERRAIN_X * 2];
static float tex_coords[TERRAIN_X * TERRAIN_Y * 2];
static float vertices[TERRAIN_X * TERRAIN_Y * 3];
static float colour[TERRAIN_X * TERRAIN_Y * 3];

static model_t *rock2;
static model_t *rock3;
static model_t *horse;
static model_t *tree;

static horse_t mdl[NUM_MODELS] = {
	// Trees.
	{ 0, 64, 18 * TERRAIN_SCALE_X, 0, 46 * TERRAIN_SCALE_Z, &tree },
	{ 0, 128, 33 * TERRAIN_SCALE_X, 0, 25 * TERRAIN_SCALE_Z, &tree },
	{ 0, 256, 35 * TERRAIN_SCALE_X, 0, 23 * TERRAIN_SCALE_Z, &tree },
	{ 0, 32, 33 * TERRAIN_SCALE_X, 0, 22 * TERRAIN_SCALE_Z, &tree },
	{ 0, 80, 33 * TERRAIN_SCALE_X, 0, 22 * TERRAIN_SCALE_Z, &tree },

	{ 0, 96, 35 * TERRAIN_SCALE_X, 0, 21 * TERRAIN_SCALE_Z, &tree },
	{ 0, 144, 35 * TERRAIN_SCALE_X, 0, 26 * TERRAIN_SCALE_Z, &tree },
	{ 0, 200, 38 * TERRAIN_SCALE_X, 0, 22 * TERRAIN_SCALE_Z, &tree },
	{ 0, 310, 39 * TERRAIN_SCALE_X, 0, 25 * TERRAIN_SCALE_Z, &tree },
	{ 0, 330, 42 * TERRAIN_SCALE_X, 0, 29 * TERRAIN_SCALE_Z, &tree },

	{ 0, 340, 39 * TERRAIN_SCALE_X, 0, 27 * TERRAIN_SCALE_Z, &tree },
	{ 0, 10, 42 * TERRAIN_SCALE_X, 0, 27 * TERRAIN_SCALE_Z, &tree },
	{ 0, 0, 44 * TERRAIN_SCALE_X, 0, 26 * TERRAIN_SCALE_Z, &tree },
	{ 0, 48, 45 * TERRAIN_SCALE_X, 0, 30 * TERRAIN_SCALE_Z, &tree },
	{ 0, 56, 20 * TERRAIN_SCALE_X, 0, 44 * TERRAIN_SCALE_Z, &tree },

	{ 0, 120, 16 * TERRAIN_SCALE_X, 0, 42 * TERRAIN_SCALE_Z, &tree },
	{ 0, 228, 15 * TERRAIN_SCALE_X, 0, 49 * TERRAIN_SCALE_Z, &tree },
	{ 0, 244, 16 * TERRAIN_SCALE_X, 0, 48 * TERRAIN_SCALE_Z, &tree },
	{ 0, 192, 43 * TERRAIN_SCALE_X, 0, 52 * TERRAIN_SCALE_Z, &tree },
	{ 0, 345, 45 * TERRAIN_SCALE_X, 0, 51 * TERRAIN_SCALE_Z, &tree },

	{ 0, 215, 42 * TERRAIN_SCALE_X, 0, 49 * TERRAIN_SCALE_Z, &tree },
	{ 0, 225, 46 * TERRAIN_SCALE_X, 0, 50 * TERRAIN_SCALE_Z, &tree },
	{ 0, 170, 13 * TERRAIN_SCALE_X, 0, 18 * TERRAIN_SCALE_Z, &tree },
	{ 0, 190, 14 * TERRAIN_SCALE_X, 0, 22 * TERRAIN_SCALE_Z, &tree },
	{ 0, 264, 17 * TERRAIN_SCALE_X, 0, 44 * TERRAIN_SCALE_Z, &tree },

	{ 0, 264, 14 * TERRAIN_SCALE_X, 0, 19 * TERRAIN_SCALE_Z, &tree },

	// Horses.
	{ 0, 45, 25 * TERRAIN_SCALE_X, 0, 32 * TERRAIN_SCALE_Z, &horse },
	{ 0, 60, 24 * TERRAIN_SCALE_X, 0, 32 * TERRAIN_SCALE_Z, &horse },
	{ 0, 120, 18 * TERRAIN_SCALE_X, 0, 38 * TERRAIN_SCALE_Z, &horse },
	{ 0, 30, 18 * TERRAIN_SCALE_X, 0, 19 * TERRAIN_SCALE_Z, &horse },
	{ 0, 90, 46 * TERRAIN_SCALE_X, 0, 45 * TERRAIN_SCALE_Z, &horse },

	{ 0, 0, 51 * TERRAIN_SCALE_X, 0, 42 * TERRAIN_SCALE_Z, &horse },
	{ 0, 100, 49 * TERRAIN_SCALE_X, 0, 31 * TERRAIN_SCALE_Z, &horse },
	{ 0, 110, 34 * TERRAIN_SCALE_X, 0, 6 * TERRAIN_SCALE_Z, &horse },
	{ 0, 220, 54 * TERRAIN_SCALE_X, 0, 6 * TERRAIN_SCALE_Z, &horse },
	{ 0, 209, 54 * TERRAIN_SCALE_X, 0, 18 * TERRAIN_SCALE_Z, &horse },

	// Big rocks.
	{ 0, 30, 27 * TERRAIN_SCALE_X, 0, 28 * TERRAIN_SCALE_Z, &rock2 },
	{ 0, 90, 61 * TERRAIN_SCALE_X, 0, 46 * TERRAIN_SCALE_Z, &rock2 },

	// Little rocks.
	{ 0, 45, 46 * TERRAIN_SCALE_X, 0, 22 * TERRAIN_SCALE_Z, &rock3 },
	{ 0, 60, 49 * TERRAIN_SCALE_X, 0, 21 * TERRAIN_SCALE_Z, &rock3 },
	{ 0, 115, 8 * TERRAIN_SCALE_X, 0, 60 * TERRAIN_SCALE_Z, &rock3 },
	{ 0, 75, 43 * TERRAIN_SCALE_X, 0, 14 * TERRAIN_SCALE_Z, &rock3 },
	{ 0, 15, 32 * TERRAIN_SCALE_X, 0, 39 * TERRAIN_SCALE_Z, &rock3 },

	{ 0, 140, 6 * TERRAIN_SCALE_X, 0, 34 * TERRAIN_SCALE_Z, &rock3 },
	{ 0, 155, 7 * TERRAIN_SCALE_X, 0, 15 * TERRAIN_SCALE_Z, &rock3 }
};

// The Bezier curve path that the camera will follow.
static point_t path[MAX_POINTS] = {
	{ 32.0F * TERRAIN_SCALE_X, 62.9F * TERRAIN_SCALE_Z },
	{ 47.9F * TERRAIN_SCALE_X, 62.9F * TERRAIN_SCALE_Z },
	{ 62.9F * TERRAIN_SCALE_X, 47.9F * TERRAIN_SCALE_Z },

	{ 62.9F * TERRAIN_SCALE_X, 32.0F * TERRAIN_SCALE_Z },
	{ 62.9F * TERRAIN_SCALE_X, 15.9F * TERRAIN_SCALE_Z },
	{ 47.9F * TERRAIN_SCALE_X, 0.1F * TERRAIN_SCALE_Z },

	{ 32.0F * TERRAIN_SCALE_X, 0.1F * TERRAIN_SCALE_Z },
	{ 15.9F * TERRAIN_SCALE_X, 0.1F * TERRAIN_SCALE_Z },
	{ 0.1F * TERRAIN_SCALE_X, 15.9F * TERRAIN_SCALE_Z },

	{ 0.1F * TERRAIN_SCALE_X, 32.0F * TERRAIN_SCALE_Z },
	{ 0.1F * TERRAIN_SCALE_X, 47.9F * TERRAIN_SCALE_Z },
	{ 15.9F * TERRAIN_SCALE_X, 62.9F * TERRAIN_SCALE_Z },

	{ 32.0F * TERRAIN_SCALE_X, 62.9F * TERRAIN_SCALE_Z },
	{ 44.0F * TERRAIN_SCALE_X, 62.9F * TERRAIN_SCALE_Z },
	{ 44.0F * TERRAIN_SCALE_X, 47.9F * TERRAIN_SCALE_Z },

	{ 44.0F * TERRAIN_SCALE_X, 32.9F * TERRAIN_SCALE_Z },
	{ 44.0F * TERRAIN_SCALE_X, 15.9F * TERRAIN_SCALE_Z },
	{ 32.0F * TERRAIN_SCALE_X, 15.9F * TERRAIN_SCALE_Z },

	{ 15.9F * TERRAIN_SCALE_X, 15.9F * TERRAIN_SCALE_Z }
};

static camera_t cam = {
	0, 0, 0, 0, 0, 0, 0, 0, 0
};

static float terrain_height(float x, float z)
{
	int vertex_x, vertex_z;
	int index;

	// Return the elevation at this point.
	if(x < 0 || x >= (TERRAIN_X - 1) * TERRAIN_SCALE_X
		|| z < 0 || z >= (TERRAIN_Y - 1)  * TERRAIN_SCALE_Z)
		return 0;

	// Whole and fractional part of patch.
	x /= TERRAIN_SCALE_X;
	vertex_x = (int) x;
	x -= vertex_x;

	z /= TERRAIN_SCALE_Z;
	vertex_z = (int) z;
	z -= vertex_z;

	// Which triangle are we on?
	if(z < (1 - x)) {
		// Top left.
		index = vertex_z * TERRAIN_X + vertex_x;

		return vertices[index * 3 + 1]
			+ (vertices[(index + 1) * 3 + 1] - vertices[index * 3 + 1]) * x
			+ (vertices[(index + TERRAIN_X) * 3 + 1] - vertices[index * 3 + 1]) * z;
	}

	else {
		// Bottom right.
		index = (vertex_z + 1) * TERRAIN_X + vertex_x;

		return vertices[index * 3 + 1]
			+ (vertices[(index + 1) * 3 + 1] - vertices[index * 3 + 1]) * x
			- (vertices[(index + 1) * 3 + 1] - vertices[(index + 1 - TERRAIN_X) * 3 + 1]) * (1 - z);
	}
}

static float terrain_shade(float x, float z)
{
	// Find the closest vertex.
	int vx, vz;

	if(x >= (TERRAIN_X - 1) * TERRAIN_SCALE_Z || x < 0
		|| z >= (TERRAIN_Y - 1) * TERRAIN_SCALE_Z || z < 0)
		return 0;

	// Integer values for vertex.
	vx = (int) (x / TERRAIN_SCALE_X);
	vz = (int) (z / TERRAIN_SCALE_Z);

	// Return the vertexes colour.
	return (colour[(vz * TERRAIN_X + vx) * 3 + 0]
		+ colour[(vz * TERRAIN_X + vx) * 3 + 1]) / 2;
}

static void bezier_walk(float t, int i)
{
	point_t a, b, c, d;
	point_t t1, t2;	// Interval bisection.
	float ia, ib;

	// Anchors and control points.
	i *= 3;

	a = path[i++];
	b = path[i++];
	c = path[i++];
	d = path[i];

	cam.x = (1.0F - t) * (1.0F - t) * (1.0F - t) * a.x
		+ 3 * t * (1.0F - t) * (1.0F - t) * b.x
		+ 3 * t * t * (1.0F - t) * c.x
		+ t * t * t * d.x;

	cam.z = (1.0F - t) * (1.0F - t) * (1.0F - t) * a.y
		+ 3 * t * (1.0F - t) * (1.0F - t) * b.y
		+ 3 * t * t * (1.0F - t) * c.y
		+ t * t * t * d.y;

	// Interval bisection code starts here. Camera heading.
	ib = t + 0.1F;
	ia = t - 0.1F;

	t1.x = (1.0F - ia) * (1.0F - ia) * (1.0F - ia) * a.x
		+ 3 * ia * (1.0F - ia) * (1.0F - ia) * b.x
		+ 3 * ia * ia * (1.0F - ia) * c.x
		+ ia * ia * ia * d.x;

	t1.y = (1.0F - ia) * (1.0F - ia) * (1.0F - ia) * a.y
		+ 3 * ia * (1.0F - ia) * (1.0F - ia) * b.y
		+ 3 * ia * ia * (1.0F - ia) * c.y
		+ ia * ia * ia * d.y;

	t2.x = (1.0F - ib) * (1.0F - ib) * (1.0F - ib) * a.x
		+ 3 * ib * (1.0F - ib) * (1.0F - ib) * b.x
		+ 3 * ib * ib * (1.0F - ib) * c.x
		+ ib * ib * ib * d.x;

	t2.y = (1.0F - ib) * (1.0F - ib) * (1.0F - ib) * a.y
		+ 3 * ib * (1.0F - ib) * (1.0F - ib) * b.y
		+ 3 * ib * ib * (1.0F - ib) * c.y
		+ ib * ib * ib * d.y;

	// Tangent to the curve.
	t2.x -= t1.x;
	t2.y -= t1.y;

	// Derive the angle from its gradient.
	if(!t2.x) t2.x += 0.0001F;

	cam.yaw = (float) (atan(-t2.y / t2.x) * (360.0 / 6.282));// - 90;

	if(t2.x < 0)
		cam.yaw += 180;
}

#ifdef BUILD_DEBUG
static void bezier_draw(void)
{
	point_t a, b, c, d;
	point_t to;
	float t;
	int i, j;

	i = 0;

	while(i < MAX_POINTS - 1) {
		a = path[i++];
		b = path[i++];
		c = path[i++];
		d = path[i];

		t = 0;

		glColor3f(1.0F, 1.0F, 1.0F);
		glBegin(GL_LINE_STRIP);

		for(j = 0; j < BEZIER_LOD; j++) {
			to.x = (1.0F - t) * (1.0F - t) * (1.0F - t) * a.x
				+ 3 * t * (1.0F - t) * (1.0F - t) * b.x
				+ 3 * t * t * (1.0F - t) * c.x
				+ t * t * t * d.x;

			to.y = (1.0F - t) * (1.0F - t) * (1.0F - t) * a.y
				+ 3 * t * (1.0F - t) * (1.0F - t) * b.y
				+ 3 * t * t * (1.0F - t) * c.y
				+ t * t * t * d.y;

			t += 1.0F / BEZIER_LOD;

			glVertex3f(to.x, terrain_height(to.x, to.y), to.y);
		}

		glVertex3f(d.x, terrain_height(d.x, d.y), d.y);
		glEnd();
	}

	glColor3f(1.0F, 0.0F, 0.0F);
	glBegin(GL_POINTS);

	for(i = 0; i < MAX_POINTS; i++)
		glVertex3f(path[i].x, terrain_height(path[i].x, path[i].y), path[i].y);

	glEnd();
}
#endif

model_t * model_load(char *pathname)
{
	FILE *model;
	model_t *m;

	if(!(m = (model_t *) malloc(sizeof(model_t))))
		return 0;

	if(!(model = fopen(pathname, "rb"))) {
		free(m);
		return 0;
	}

	fread(&m->num_vertices, sizeof(unsigned short), 1, model);
	fread(&m->num_triangles, sizeof(unsigned short), 1, model);

	if(!(m->vertex = (vertex_t *) malloc(sizeof(vertex_t) * m->num_vertices))) {
		fclose(model);
		free(m);
		return 0;
	}

	if(!(m->triangle = (triangle_t *) malloc(sizeof(triangle_t) * m->num_triangles))) {
		fclose(model);
		free(m->vertex);
		free(m);
		return 0;
	}

	fread(m->vertex, sizeof(vertex_t), m->num_vertices, model);
	fread(m->triangle, sizeof(triangle_t), m->num_triangles, model);
	fclose(model);

	return m;
}

void model_free(model_t * m)
{
	if(m) {
		free(m->triangle);
		free(m->vertex);

		m = 0;
	}
}

static void model_draw(model_t *m)
{
	unsigned int i;

	glBegin(GL_TRIANGLES);

	for(i = 0; i < m->num_triangles; i++) {
		glTexCoord2d(m->triangle[i].u[0], m->triangle[i].v[0]);
		glVertex3fv((float *) &m->vertex[m->triangle[i].vertex[0]]);
		glTexCoord2d(m->triangle[i].u[1], m->triangle[i].v[1]);
		glVertex3fv((float *) &m->vertex[m->triangle[i].vertex[1]]);
		glTexCoord2d(m->triangle[i].u[2], m->triangle[i].v[2]);
		glVertex3fv((float *) &m->vertex[m->triangle[i].vertex[2]]);
	}

	glEnd();
}

static void rohan_skybox(void)
{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	// Front face.
	glBindTexture(GL_TEXTURE_2D, texture_id(1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBegin(GL_QUADS);
	glTexCoord2d(0, 1);
	glVertex3f(-200, -70, -200);
	glTexCoord2d(1, 1);
	glVertex3f(200, -70, -200);
	glTexCoord2d(1, 0);
	glVertex3f(200, 80, -200);
	glTexCoord2d(0, 0);
	glVertex3f(-200, 80, -200);
	glEnd();

	// Back face.
	glBindTexture(GL_TEXTURE_2D, texture_id(2));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBegin(GL_QUADS);
	glTexCoord2d(1, 0);
	glVertex3f(-200, 80, 200);
	glTexCoord2d(0, 0);
	glVertex3f(200, 80, 200);
	glTexCoord2d(0, 1);
	glVertex3f(200, -70, 200);
	glTexCoord2d(1, 1);
	glVertex3f(-200, -70, 200);
	glEnd();

	// Left face.
	glBindTexture(GL_TEXTURE_2D, texture_id(3));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBegin(GL_QUADS);
	glTexCoord2d(0, 1);
	glVertex3f(-200, -70, 200);
	glTexCoord2d(1, 1);
	glVertex3f(-200, -70, -200);
	glTexCoord2d(1, 0);
	glVertex3f(-200, 80, -200);
	glTexCoord2d(0, 0);
	glVertex3f(-200, 80, 200);
	glEnd();

	// Right face.
	glBindTexture(GL_TEXTURE_2D, texture_id(4));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBegin(GL_QUADS);
	glTexCoord2d(1, 0);
	glVertex3f(200, 80, 200);
	glTexCoord2d(0, 0);
	glVertex3f(200, 80, -200);
	glTexCoord2d(0, 1);
	glVertex3f(200, -70, -200);
	glTexCoord2d(1, 1);
	glVertex3f(200, -70, 200);
	glEnd();

	// Top face.
	glBindTexture(GL_TEXTURE_2D, texture_id(6));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBegin(GL_QUADS);
	glTexCoord2d(1, 1);
	glVertex3f(-200, 80, -200);
	glTexCoord2d(1, 0);
	glVertex3f(200, 80, -200);
	glTexCoord2d(0, 0);
	glVertex3f(200, 80, 200);
	glTexCoord2d(0, 1);
	glVertex3f(-200, 80, 200);
	glEnd();

	// Bottom face.
	glBindTexture(GL_TEXTURE_2D, texture_id(5));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBegin(GL_QUADS);
	glTexCoord2d(0, 10);
	glVertex3f(-200, -70, 200);
	glTexCoord2d(10, 10);
	glVertex3f(200, -70, 200);
	glTexCoord2d(10, 0);
	glVertex3f(200, -70, -200);
	glTexCoord2d(0, 0);
	glVertex3f(-200, -70, -200);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

static float rohan_opacity(unsigned long time, unsigned long base_time)
{
	// Fading in.
	if(time - base_time < 1000)
		return 0.001F * (time - base_time);

	// Fading out.
	if(time - base_time > 4000)
		return 0.001F * (base_time + 5000 - time);

	// Fully opaque.
	return 1;
}

static float drift_text(int starting)
{
	if(starting < 0 || starting > 5000)
		return 0;

	if(starting < 1000)
		return 0.04F * (starting - 1000);
	if(starting > 4000)
		return 0.04F * (starting - 4000);

	return 0;
}

void rohan_terrain_process(unsigned long time)
{
	int z;

	glClear(GL_DEPTH_BUFFER_BIT);

	graphics_perspective();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	cam.y = terrain_height(cam.x, cam.z) + 10;

	glRotatef(-cam.yaw, 0, 1, 0);

	// Draw the skybox.
	glShadeModel(GL_FLAT);
	rohan_skybox();
	glShadeModel(GL_SMOOTH);

	glTranslatef(-cam.x, -cam.y, -cam.z);

	// Depth buffer conventions.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Enable texturing.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_id(5));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	for(z = 0; z < TERRAIN_Y - 1; z++)
		glDrawElements(GL_TRIANGLE_STRIP, TERRAIN_X * 2, GL_UNSIGNED_INT, &indices[z * TERRAIN_X * 2]);

#ifdef BUILD_DEBUG
	// Draw the camera path.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	bezier_draw();
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
#endif

	glShadeModel(GL_FLAT);
	glMatrixMode(GL_MODELVIEW);

	for(z = 0; z < NUM_MODELS; z++) {
		// Transform the models into camera space.
		glLoadIdentity();
		glRotatef(-cam.yaw, 0, 1, 0);
		glTranslatef(-cam.x + mdl[z].x, -cam.y + mdl[z].y, -cam.z + mdl[z].z);
		glRotatef(mdl[z].yaw, 0, 1, 0);
		glColor3f(mdl[z].shade, mdl[z].shade, mdl[z].shade);

		if(*mdl[z].frame == horse)
			glBindTexture(GL_TEXTURE_2D, texture_id(7));
		else if(*mdl[z].frame == tree)
			glBindTexture(GL_TEXTURE_2D, texture_id(9));
		else
			glBindTexture(GL_TEXTURE_2D, texture_id(8));

		model_draw(*mdl[z].frame);
	}

	// Add some water to the terrain.
	glLoadIdentity();
	glRotatef(-cam.yaw, 0, 1, 0);
	glTranslatef(-cam.x, -cam.y, -cam.z);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1, 1, 1, 0.5F);

	glBindTexture(GL_TEXTURE_2D, texture_id(10));

	glBegin(GL_QUADS);
	glTexCoord2d(0, 0);
	glVertex3f(0, 17.9F, (TERRAIN_Y - 1) * TERRAIN_SCALE_Z);
	glTexCoord2d(16, 0);
	glVertex3f((TERRAIN_X - 1) * TERRAIN_SCALE_X, 17.9F, (TERRAIN_Y - 1) * TERRAIN_SCALE_Z);
	glTexCoord2d(16, 16);
	glVertex3f((TERRAIN_X - 1) * TERRAIN_SCALE_X, 17.9F, 0);
	glTexCoord2d(0, 16);
	glVertex3f(0, 17.9F, 0);
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	bezier_walk(cam.delta, cam.point);
	cam.delta += 0.00017F * (time - cam.last);
	cam.last = time;

	if(cam.delta > 1.0F) {
		cam.delta -= 1.0F;
		cam.point++;

		if(cam.point >= (MAX_POINTS - 1) / 3) {
			cam.delta = 0;
			cam.point = 0;
		}
	}

	graphics_orthographic();
	text_colour (1.0,1.0,1.0);
	if(time < 7000 && time > 2000)
		text_print(320 + drift_text(time - 2000), VIEW_SIZE_Y - 55,
			"Finally in Sweden....", rohan_opacity(time, 2000));

	else if(time < 14000 && time > 9000)
		text_print(320 - drift_text(time - 9000), VIEW_SIZE_Y - 55,
			"Lets look around....outside the hall", rohan_opacity(time, 9000));

	else if(time < 21000 && time > 16000)
		text_print(320 + drift_text(time - 16000), VIEW_SIZE_Y - 55,
			"Where is the snow ? ", rohan_opacity(time, 16000));

	else if(time < 28000 && time > 23000)
		text_print(320 - drift_text(time - 23000), VIEW_SIZE_Y - 55,
			"Danes lovvvvvvve Sweden!", rohan_opacity(time, 23000));

	glDisable(GL_TEXTURE_2D);
}

void rohan_terrain_end(void)
{
	texture_free(1);
	texture_free(2);
	texture_free(3);
	texture_free(4);
	texture_free(5);
	texture_free(6);
	texture_free(7);
	texture_free(8);
	texture_free(9);
	texture_free(10);

	model_free(horse);
	model_free(rock2);
	model_free(rock3);
	model_free(tree);	// I want to free a tree!

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

/*
The terrain is pre-processed and a colour is produced
at each vertex, given the angle from that vertices
normal to the sun. I have tried to make the virtual
sun position correspond to the sun seen in Justin's
skybox.
*/

static int get_vertices_from_terrain(char *pathname)
{
	const float sun[] = { -500, 1000, 500 };
	float *normals, *tmp_norm, vec_right[3], vec_down[3], vec_left[3], vec_up[3];
	unsigned char *buffer;
	float magnitude;
	FILE *terrain;
	int w, x, y, z;

	if(!(terrain = fopen(pathname, "rb")))
		return 0;

	// Allocate storage for the heightmap.
	if(!(buffer = (unsigned char *) malloc(sizeof(unsigned char) * TERRAIN_X * TERRAIN_Y)))
		return 0;

	// Allocate temporary storage for intermediate normals.
	if(!(tmp_norm = (float *) malloc(sizeof(float) * TERRAIN_X * TERRAIN_Y * 3)))
		return 0;

	// Allocate temporary storage for intermediate normals.
	if(!(normals = (float *) malloc(sizeof(float) * TERRAIN_X * TERRAIN_Y * 3)))
		return 0;

	fread(buffer, TERRAIN_X * TERRAIN_Y, 1, terrain);
	fclose(terrain);

	// Extrude heightmap into vertex positions.
	for(z = y = w = 0; z < TERRAIN_Y; z++) {
		for(x = 0; x < TERRAIN_X; x++) {
			vertices[y++] = (float) x * TERRAIN_SCALE_X;
			vertices[y++] = (float) buffer[w++] * TERRAIN_SCALE_Y;
			vertices[y++] = (float) z * TERRAIN_SCALE_Z;
		}
	}

	// Generate texture coordinates.
	for(z = y = 0; z < TERRAIN_Y; z++) {
		for(x = 0; x < TERRAIN_X; x++) {
			tex_coords[y++] = 0.25F * x;
			tex_coords[y++] = 0.25F * z;
		}
	}

	// Calculate vertex colour from terrain height.
	// This will be modulated with software lighting calculations later.
	for(z = y = w = 0; z < TERRAIN_Y; z++) {
		for(x = 0; x < TERRAIN_X; x++) {
			colour[y++] = (1.0F / 64) * buffer[w];
			colour[y++] = 0.8F + (0.2F / 64) * buffer[w++];
			colour[y++] = 0;
		}
	}

	// Build vertex lists for faster rendering.
	for(z = y = 0; z < TERRAIN_Y - 1; z++) {
		for(x = 0; x < TERRAIN_X; x++) {
			indices[y++] = (z + 0) * TERRAIN_X + x;
			indices[y++] = (z + 1) * TERRAIN_X + x;
		}
	}

	// Construct face normals for each vertex.
	// These will be turned into vertex normals later.
	for(z = y = 0; z < TERRAIN_Y; z++) {
		for(x = 0; x < TERRAIN_X; x++) {
			if(x == TERRAIN_X - 1) {
				vec_right[0] = TERRAIN_SCALE_X;
				vec_right[1] = 0;
				vec_right[2] = 0;
			}

			else
				vector_subtract(vec_right, &vertices[(z * TERRAIN_X + x + 1) * 3], &vertices[y]);

			if(z == TERRAIN_Y - 1) {
				vec_down[0] = 0;
				vec_down[1] = 0;
				vec_down[2] = TERRAIN_SCALE_Z;
			}

			else
				vector_subtract(vec_down, &vertices[((z + 1) * TERRAIN_X + x) * 3], &vertices[y]);

			// Cross product gives normal. Clockwise.
			cross_product(&tmp_norm[y], vec_down, vec_right);

			// Normalize the temporary normal.
			magnitude = tmp_norm[y + 0] * tmp_norm[y + 0]
				+ tmp_norm[y + 1] * tmp_norm[y + 1]
				+ tmp_norm[y + 2] * tmp_norm[y + 2];

			if(magnitude) {
				magnitude = (float) sqrt(magnitude);
				magnitude = 1.0F / magnitude;

				tmp_norm[y + 0] *= magnitude;
				tmp_norm[y + 1] *= magnitude;
				tmp_norm[y + 2] *= magnitude;
			}

			y += 3;
		}
	}

	// Average face normals, to get vertex normals.
	for(z = y = 0; z < TERRAIN_Y; z++) {
		for(x = 0; x < TERRAIN_X; x++) {
			// Substitue perfect normals for those off the edges.
			if(x == TERRAIN_X - 1) {
				vec_right[0] = 0;
				vec_right[1] = 1;
				vec_right[2] = 0;
			}

			else {
				vec_right[0] = tmp_norm[y + 3];
				vec_right[1] = tmp_norm[y + 4];
				vec_right[2] = tmp_norm[y + 5];
			}

			if(x == 0) {
				vec_left[0] = 0;
				vec_left[1] = 1;
				vec_left[2] = 0;
			}

			else {
				vec_left[0] = tmp_norm[y - 3];
				vec_left[1] = tmp_norm[y - 2];
				vec_left[2] = tmp_norm[y - 1];
			}

			if(z == TERRAIN_Y - 1) {
				vec_down[0] = 0;
				vec_down[1] = 1;
				vec_down[2] = 0;
			}

			else {
				vec_down[0] = tmp_norm[y + TERRAIN_X * 3 + 0];
				vec_down[1] = tmp_norm[y + TERRAIN_X * 3 + 1];
				vec_down[2] = tmp_norm[y + TERRAIN_X * 3 + 2];
			}

			if(z == 0) {
				vec_up[0] = 0;
				vec_up[1] = 1;
				vec_up[2] = 0;
			}

			else {
				vec_up[0] = tmp_norm[y - TERRAIN_X * 3 + 0];
				vec_up[1] = tmp_norm[y - TERRAIN_X * 3 + 1];
				vec_up[2] = tmp_norm[y - TERRAIN_X * 3 + 2];
			}

			// Sum all five surrounding normals.
			normals[y + 0] = vec_left[0] + vec_up[0] + vec_right[0]
				+ vec_down[0] + tmp_norm[y + 0];
			normals[y + 1] = vec_left[1] + vec_up[1] + vec_right[1]
				+ vec_down[1] + tmp_norm[y + 1];
			normals[y + 2] = vec_left[2] + vec_up[2] + vec_right[2]
				+ vec_down[2] + tmp_norm[y + 2];

			// Renormalize. Vector magnitude now 5.0F.
			normals[y + 0] *= 0.2F;
			normals[y + 1] *= 0.2F;
			normals[y + 2] *= 0.2F;

			y += 3;
		}
	}

	// Calculate angle to the sun, and shade accordingly.
	for(z = y = 0; z < TERRAIN_Y; z++) {
		for(x = 0; x < TERRAIN_X; x++) {
			// Direction to the sun from this vertex.
			vector_subtract(vec_up, sun, &vertices[y]);

			// Normalize the normal.
			magnitude = vec_up[0] * vec_up[0] + vec_up[1] * vec_up[1]
				+ vec_up[2] * vec_up[2];

			if(magnitude) {
				magnitude = (float) sqrt(magnitude);
				magnitude = 1.0F / magnitude;

				vec_up[0] *= magnitude;
				vec_up[1] *= magnitude;
				vec_up[2] *= magnitude;
			}

			// Take the angle between the two.
			magnitude = vec_up[0] * normals[y + 0] + vec_up[1] * normals[y + 1]
				+ vec_up[2] * normals[y + 2];

			if(magnitude > 0.0F) {
				colour[y + 0] *= magnitude;
				colour[y + 1] *= magnitude;
				colour[y + 2] *= magnitude;
			}

			else {
				colour[y + 0] = 0;
				colour[y + 1] = 0;
				colour[y + 2] = 0;
			}

			y += 3;
		}
	}

	// Clean up what we allocated.
	free(tmp_norm);
	free(normals);
	free(buffer);

	return 1;
}

void rohan_terrain_setup(void)
{
	int i, x, y;

	// Load the terrain heightmap.
	if(!(get_vertices_from_terrain("data\\rohan.raw")))
		exit(1);

	// Load the skybox textures.             //new //old
    if(!texture_load("data\\right.bmp", 1)) //right //front
		exit(1);
	if(!texture_load("data\\left.bmp", 2)) // left //back
		exit(1);
	if(!texture_load("data\\front.bmp", 3)) // front //left
		exit(1);
	if(!texture_load("data\\back.bmp", 4)) //back //right
		exit(1);
	if(!texture_load("data\\bottom.bmp", 5))
		exit(1);
	if(!texture_load("data\\top.bmp", 6))
		exit(1);
	/*if(!texture_load("data\\Ruins_ft.bmp", 1))
		exit(1);
	if(!texture_load("data\\Ruins_bk.bmp", 2))
		exit(1);
	if(!texture_load("data\\Ruins_lf.bmp", 3))
		exit(1);
	if(!texture_load("data\\Ruins_rt.bmp", 4))
		exit(1);
	if(!texture_load("data\\Ruins_dn.bmp", 5))
		exit(1);
	if(!texture_load("data\\Ruins_up.bmp", 6))
		exit(1);
		*/
	//if(!texture_load("data\\horse.bmp", 7))
	//	exit(1);
	if(!texture_load("data\\rock.bmp", 8))
		exit(1);
	if(!texture_load("data\\tree.bmp", 9))
		exit(1);
	if(!texture_load("data\\water.bmp", 10))
		exit(1);

	if(!(horse = model_load("data\\horse.mod")))
		exit(1);

	if(!(rock2 = model_load("data\\rock2.mod")))
		exit(1);

	if(!(rock3 = model_load("data\\rock3.mod")))
		exit(1);

	if(!(tree = model_load("data\\tree.mod")))
		exit(1);

	// Place and shade all the horses.
	for(i = 0; i < NUM_MODELS; i++) {
		mdl[i].y = terrain_height(mdl[i].x, mdl[i].z);
		mdl[i].shade = terrain_shade(mdl[i].x, mdl[i].z);

// Experiment. Attempt to add adhoc shadows to objects.
		x = (int) (mdl[i].x / TERRAIN_SCALE_X);
		y = (int) (mdl[i].z / TERRAIN_SCALE_Z);
		y = y * TERRAIN_X + x;
		y *= 3;

		if(mdl[i].frame != &horse) {
			// Rocks look good with harsh shadows.
			colour[y + 0] -= 0.5F;
			colour[y + 1] -= 0.5F;
			colour[y + 2] -= 0.5F;
		}

		else {
			// Horses look odd with harsh shadows.
			colour[y + 0] -= 0.25F;
			colour[y + 1] -= 0.25F;
			colour[y + 2] -= 0.25F;
		}
// Ends experiment.
	}

	// Enable buffers for the terrain.
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	// Set geometry arrays up.
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer(3, GL_FLOAT, 0, colour);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
}