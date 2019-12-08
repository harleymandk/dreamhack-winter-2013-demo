#include <windows.h>
#include <gl\glu.h>
#include <gl\gl.h>
#include <gl\glext.h>	// GL 1.2 Post texture specular.
#include <math.h>

#include "common.h"
#include "graphics.h"
#include "texture.h"
#include "demo.h"
#include "text.h"

#define RING_THICKNESS 6.0F
#define RING_RADIUS 50.0F
#define RING_RATIO 1.9F
#define RING_SEGS_H 48
#define RING_SEGS_V 16

#define MID_X (VIEW_SIZE_X >> 1)
#define MID_Y (VIEW_SIZE_Y >> 1)

typedef struct
{
	float colour[3];
	float angle;
} ring_t;

// Ring data.
static float norm[RING_SEGS_H * RING_SEGS_V * 3];
static float ring[RING_SEGS_H * RING_SEGS_V * 3];
static float text_u[RING_SEGS_H * RING_SEGS_V];
static float text_v[RING_SEGS_H * RING_SEGS_V];

// Ring colours.
static ring_t elven = { { 0.961F, 0.699F, 0.242F }, 45.0F };
static ring_t dwarven = { { 0.586F, 0.590F, 0.733F }, 90.0F };
static ring_t mortals = { { 0.958F, 0.563F, 0.216F }, 10.0F };
static ring_t sauron = { { 0.961F, 0.699F, 0.242F }, -35.0F };

// Specular lighting.
static float ring_specular[] = { 1.0F, 1.0F, 1.0F, 1.0F };
static float ambient_light[] = { 0.5F, 0.5F, 0.5F, 1.0F };
static float light_position[] = { 0, 0, 100.0F, 0.0F };
static float colour_white[] = { 1.0F, 1.0F, 1.0F, 1.0F };
static float ring_shininess[] = { 9.0F };

// Ring motion.
static float radius = 150.0F;
static float angle =  0;

static void generate_ring(void)
{
	float *current1, *current2, *current3;
	float normals[RING_SEGS_V * RING_SEGS_H][3];
	float vector1[3], vector2[3];
	float angle_ac, angle_as;
	float angle_bc, angle_bs;
	int i, j, k, l, m, n;

	current1 = ring;
	current2 = text_u;
	current3 = text_v;

	// Produce vertex positions.
	for(i = 0; i < RING_SEGS_V; i++) {
		angle_bs = (float) sin(6.282F / RING_SEGS_V * i) * RING_RATIO * RING_THICKNESS;
		angle_bc = (float) cos(6.282F / RING_SEGS_V * i) * RING_THICKNESS;

		for(j = 0; j < RING_SEGS_H; j++) {
			angle_ac = (float) cos(6.282F / RING_SEGS_H * j);
			angle_as = (float) sin(6.282F / RING_SEGS_H * j);

			*current1++ = RING_RADIUS * angle_ac + angle_bc * angle_ac;
			*current1++ = angle_bs;
			*current1++ = RING_RADIUS * angle_as + angle_bc * angle_as;
			*current2++ = j * 4.0F / RING_SEGS_H;
			*current3++ = -0.5F + i * 1.0F / RING_SEGS_V;
		}
	}

	current1 = (float *) normals;

	// Produce normals for lighting.
	for(i = 0; i < RING_SEGS_V - 1; i++) {
		for(j = 0; j < RING_SEGS_H - 1; j++) {
			k = (i * RING_SEGS_H + j) * 3;

			vector_subtract(vector2, &ring[((i + 1) * RING_SEGS_H + j) * 3], &ring[k]);
			vector_subtract(vector1, &ring[k], &ring[k + 3]);
			cross_product(current1, vector1, vector2);
			current1 += 3;
		}

		k = (i * RING_SEGS_H + j) * 3;

		vector_subtract(vector2, &ring[((i + 1) * RING_SEGS_H + j) * 3], &ring[k]);
		vector_subtract(vector1, &ring[k], &ring[i * RING_SEGS_H * 3]);
		cross_product(current1, vector1, vector2);
		current1 += 3;
	}

	for(j = 0; j < RING_SEGS_H - 1; j++) {
		k = (i * RING_SEGS_H + j) * 3;

		vector_subtract(vector2, &ring[j * 3], &ring[k]);
		vector_subtract(vector1, &ring[k], &ring[k + 3]);
		cross_product(current1, vector1, vector2);
		current1 += 3;
	}

	k = (i * RING_SEGS_H + j) * 3;

	vector_subtract(vector2, &ring[j * 3], &ring[k]);
	vector_subtract(vector1, &ring[k], &ring[i * RING_SEGS_H * 3]);
	cross_product(current1, vector1, vector2);
	current1 += 3;

	// Normalize normals. Currently surface normals.
	for(i = 0; i < RING_SEGS_H * RING_SEGS_V; i++) {
		angle_ac = normals[i][0] * normals[i][0]
			+ normals[i][1] * normals[i][1]
			+ normals[i][2] * normals[i][2];

		if(angle_ac) {
			angle_as = (float) sqrt(angle_ac);
			angle_ac = 1 / angle_as;

			normals[i][0] *= angle_ac;
			normals[i][1] *= angle_ac;
			normals[i][2] *= angle_ac;
		}
	}

	// Average normals for correct shading. Gives vertex normals.
	for(i = 0; i < RING_SEGS_V; i++) {
		l = (i - 1) * RING_SEGS_H;
		m = (i + 1) * RING_SEGS_H;

		if(i == 0)
			l = (RING_SEGS_V - 1) * RING_SEGS_H;
		else if(i == RING_SEGS_V - 1)
			m = 0;

		for(j = 0; j < RING_SEGS_H; j++) {
			k = i * RING_SEGS_H + j - 1;
			n =  i * RING_SEGS_H + j + 1;

			if(j == 0)
				k = i * RING_SEGS_H + (RING_SEGS_H - 1);
			else if(j == RING_SEGS_H - 1)
				n = i * RING_SEGS_H;

			norm[(i * RING_SEGS_H + j) * 3 + 0] = normals[k][0] + normals[l][0] + normals[m][0]
				+ normals[n][0] + normals[i * RING_SEGS_H + j][0];

			norm[(i * RING_SEGS_H + j) * 3 + 1] = normals[k][1] + normals[l][1] + normals[m][1]
				+ normals[n][1] + normals[i * RING_SEGS_H + j][1];

			norm[(i * RING_SEGS_H + j) * 3 + 2] = normals[k][2] + normals[l][2] + normals[m][2]
				+ normals[n][2] + normals[i * RING_SEGS_H + j][2];

			// Renormalize. Vector magnitude now 5.0F.
			norm[(i * RING_SEGS_H + j) * 3 + 0] *= 0.2F;
			norm[(i * RING_SEGS_H + j) * 3 + 1] *= 0.2F;
			norm[(i * RING_SEGS_H + j) * 3 + 2] *= 0.2F;

			l++;
			m++;
		}
	}
}

static void draw_ring(void)
{
	glPushMatrix();
	//glScalef(1.0f, 1.0f, 1.0f);
	glTranslatef(0.0f, 10.0f, 10.0f);
  
    glBegin(GL_TRIANGLES);
	
	glColor3ub(0, 255, 0);
	glVertex3f(0.0f, 0.0f, 60.0f);
    glColor3ub(0, 115, 0);
	glVertex3f(-15.0f, 0.0f,30.0f);
    glColor3ub(0, 115, 0);
	glVertex3f(15.0f,0.0f,30.0f);



        // Black
		 glColor3ub(0, 115, 0);
        glVertex3f(15.0f,0.0f,30.0f);
		 glColor3ub(0, 115, 0);
        glVertex3f(0.0f, 15.0f, 30.0f);
		glColor3ub(0, 255, 0);
        glVertex3f(0.0f, 0.0f, 60.0f);
	
        // Red
		glColor3ub(0, 255, 0);
        glVertex3f(0.0f, 0.0f, 60.0f);
		glColor3ub(0, 115, 0);
        glVertex3f(0.0f, 15.0f, 30.0f);
		glColor3ub(0, 115, 0);
        glVertex3f(-15.0f,0.0f,30.0f);


	// Body of the Plane ////////////////////////
        // Green
        glColor3ub(120,120,120);
        glVertex3f(-15.0f,0.0f,30.0f);
        glVertex3f(0.0f, 15.0f, 30.0f);
        glVertex3f(0.0f, 0.0f, -56.0f);
		
        glColor3ub(155,155,155);
        glVertex3f(0.0f, 0.0f, -56.0f);
        glVertex3f(0.0f, 15.0f, 30.0f);
        glVertex3f(15.0f,0.0f,30.0f);	
	
        glColor3ub(155, 155, 155);
        glVertex3f(15.0f,0.0f,30.0f);
        glVertex3f(-15.0f, 0.0f, 30.0f);
        glVertex3f(0.0f, 0.0f, -56.0f);
	
	///////////////////////////////////////////////
	// Left wing
	// Large triangle for bottom of wing
        glColor3ub(128,0,128);
        glVertex3f(0.0f,2.0f,27.0f);
        glVertex3f(-60.0f, 2.0f, -8.0f);
        glVertex3f(60.0f, 2.0f, -8.0f);
	
        glColor3ub(64,0,64);
        glVertex3f(60.0f, 2.0f, -8.0f);
        glVertex3f(0.0f, 7.0f, -8.0f);
        glVertex3f(0.0f,2.0f,27.0f);
	
        glColor3ub(192,0,192);
        glVertex3f(60.0f, 2.0f, -8.0f);
        glVertex3f(-60.0f, 2.0f, -8.0f);
        glVertex3f(0.0f,7.0f,-8.0f);
	
	// Other wing top section
        glColor3ub(0,64,64);
        glVertex3f(0.0f,2.0f,27.0f);
        glVertex3f(0.0f, 7.0f, -8.0f);
        glVertex3f(-60.0f, 2.0f, -8.0f);
	
	// Tail section///////////////////////////////
	// Bottom of back fin
        glColor3ub(155,128,155);
        glVertex3f(-30.0f, -0.50f, -57.0f);
        glVertex3f(30.0f, -0.50f, -57.0f);
        glVertex3f(0.0f,-0.50f,-40.0f);
	
        // top of left side
        glColor3ub(255,128,128);
        glVertex3f(0.0f,-0.5f,-40.0f);
        glVertex3f(30.0f, -0.5f, -57.0f);
        glVertex3f(0.0f, 4.0f, -57.0f);
	
        // top of right side
        glColor3ub(255,128,128);
        glVertex3f(0.0f, 4.0f, -57.0f);
        glVertex3f(-30.0f, -0.5f, -57.0f);
        glVertex3f(0.0f,-0.5f,-40.0f);
	
        // back of bottom of tail
        glColor3ub(255,255,255);
        glVertex3f(30.0f,-0.5f,-57.0f);
        glVertex3f(-30.0f, -0.5f, -57.0f);
        glVertex3f(0.0f, 4.0f, -57.0f);
	
        // Top of Tail section left
        glColor3ub(120,120,120);
        glVertex3f(0.0f,0.5f,-40.0f);
        glVertex3f(3.0f, 0.5f, -57.0f);
        glVertex3f(0.0f, 25.0f, -65.0f);
	
        glColor3ub(155,155,155);
        glVertex3f(0.0f, 25.0f, -65.0f);
        glVertex3f(-3.0f, 0.5f, -57.0f);
        glVertex3f(0.0f,0.5f,-40.0f);

        // Back of horizontal section
        glColor3ub(128,128,128);
        glVertex3f(3.0f,0.5f,-57.0f);
        glVertex3f(-3.0f, 0.5f, -57.0f);
        glVertex3f(0.0f, 25.0f, -65.0f);
	
    glEnd(); // Of Jet
glPopMatrix();	

    }

/*
	int i, j, k, l;

	glBegin(GL_TRIANGLE_STRIP);

	for(i = 0; i < RING_SEGS_V - 1; i++) {
		for(j = 0; j < RING_SEGS_H; j++) {
			k = ((i + 0) * RING_SEGS_H + j) * 3;
			l = ((i + 1) * RING_SEGS_H + j) * 3;

			glNormal3fv(&norm[k]);
			glVertex3fv(&ring[k]);
			glNormal3fv(&norm[l]);
			glVertex3fv(&ring[l]);
		}

		k = (i + 0) * RING_SEGS_H * 3;
		l = (i + 1) * RING_SEGS_H * 3;

		glNormal3fv(&norm[k]);
		glVertex3fv(&ring[k]);
		glNormal3fv(&norm[l]);
		glVertex3fv(&ring[l]);
	}

	for(j = 0; j < RING_SEGS_H; j++) {
		k = (i * RING_SEGS_H + j) * 3;
		l = j * 3;

		glNormal3fv(&norm[k]);
		glVertex3fv(&ring[k]);
		glNormal3fv(&norm[l]);
		glVertex3fv(&ring[l]);
	}

	k = i * RING_SEGS_H * 3;
	l = 0;

	glNormal3fv(&norm[k]);
	glVertex3fv(&ring[k]);
	glNormal3fv(&norm[l]);
	glVertex3fv(&ring[l]);

	glEnd();
}

*/

static void draw_ring_textured(void)
{
	int i, j, k, l;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_id(2));

	glBegin(GL_TRIANGLE_STRIP);

	for(i = 0; i < RING_SEGS_V - 1; i++) {
		for(j = 0; j < RING_SEGS_H; j++) {
			k = (i + 0) * RING_SEGS_H + j;
			l = (i + 1) * RING_SEGS_H + j;

			glTexCoord2f(text_u[k], text_v[k]);
			glNormal3fv(&norm[k * 3]);
			glVertex3fv(&ring[k * 3]);
			glTexCoord2f(text_u[l], text_v[l]);
			glNormal3fv(&norm[l * 3]);
			glVertex3fv(&ring[l * 3]);

		}

		k = (i + 0) * RING_SEGS_H;
		l = (i + 1) * RING_SEGS_H;

		glTexCoord2f(4.0F, text_v[k]);
		glNormal3fv(&norm[k * 3]);
		glVertex3fv(&ring[k * 3]);
		glTexCoord2f(4.0F, text_v[l]);
		glNormal3fv(&norm[l * 3]);
		glVertex3fv(&ring[l * 3]);
	}

	for(j = 0; j < RING_SEGS_H; j++) {
		k = (i + 0) * RING_SEGS_H + j;
		l = (0 + 0) * RING_SEGS_H + j;

		glTexCoord2f(text_u[k], text_v[k]);
		glNormal3fv(&norm[k * 3]);
		glVertex3fv(&ring[k * 3]);
		glTexCoord2f(text_u[l], 0.5F);
		glNormal3fv(&norm[l * 3]);
		glVertex3fv(&ring[l * 3]);
	}

	k = i * RING_SEGS_H;
	l = 0;

	glTexCoord2f(4.0F, 0.5F);
	glNormal3fv(&norm[k * 3]);
	glVertex3fv(&ring[k * 3]);
	glTexCoord2f(4.0F, 0.5F);
	glNormal3fv(&norm[l * 3]);
	glVertex3fv(&ring[l * 3]);

	glEnd();

	glDisable(GL_TEXTURE_2D);
}

static void draw_background(unsigned long elapsed)
{
	// Draw the swirling background in two dimensions.
	graphics_orthographic();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texture_id(1));
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glTranslatef(0.0002F * elapsed, 0.0001F * elapsed, 0);
	glRotatef(0.001F * elapsed, 0, 0, 1);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	glTexCoord2f(2.5F, 0);
	glVertex2f(VIEW_SIZE_X, 0);
	glTexCoord2f(2.5F, 1.875F);
	glVertex2f(VIEW_SIZE_X, VIEW_SIZE_Y);
	glTexCoord2f(0, 1.875F);
	glVertex2f(0, VIEW_SIZE_Y);
	glEnd();

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glTranslatef(0.0001F * elapsed, 0.0002F * elapsed, 0);
	glRotatef(0.002F * elapsed, 0, 0, -1);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	glTexCoord2f(1.5F, 0);
	glVertex2f(VIEW_SIZE_X, 0);
	glTexCoord2f(1.5F, 0.8F);
	glVertex2f(VIEW_SIZE_X, VIEW_SIZE_Y);
	glTexCoord2f(0, 0.8F);
	glVertex2f(0, VIEW_SIZE_Y);
	glEnd();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

static float fade_text(int starting)
{
	if(starting < 0 || starting > 4000)
		return 0;

	if(starting < 1000)
		return 0.001F * starting;

	if(starting > 3000)
		return 0.001F * (4000 - starting);

	return 1;
}

static float drift_text(int starting)
{
	if(starting < 0 || starting > 4000)
		return 0;

	if(starting < 1000)
		return 0.04F * (starting - 1000);
	if(starting > 3000)
		return 0.04F * (starting - 3000);

	return 0;
}

void ring_poem_process(unsigned long time)
{
	float opacity, drift;
	int i;

	glClear(GL_DEPTH_BUFFER_BIT);

	draw_background(time);
	graphics_perspective();

	// Depth buffer filling conventions.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Backface culling methods.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glShadeModel(GL_SMOOTH);

	// Specular lighting.
	glMaterialfv(GL_FRONT, GL_SHININESS, ring_shininess);
	glMaterialfv(GL_FRONT, GL_SPECULAR, ring_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_SPECULAR, colour_white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, colour_white);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Three Rings for the Elven-kings under the sky.
	if(time < 10000) {
		elven.angle = 0.02F * time;
		angle = 0.00025F * time;
		radius = 150.0F;

		// Ideal radius = 500 => 150 => 500
		if(time < 2500)
			radius = 500.0F - (350.0F / 2500.0F) * time;
		else if(time > 7500)
			radius = 500.0F - (350.0F / 2500.0F) * (10000 - time);

		for(i = 0; i < 3; i++) {
			glLoadIdentity();

			glTranslatef(radius * (float) cos((6.282F / 3 * i) + angle), radius * (float) sin((6.282F / 3 * i) + angle), -400.0F);
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, elven.colour);
			glRotatef(elven.angle, 1.0F, 1.0F, 0);
			draw_ring();
		}
	}

	// 
	else if(time < 20000) {
		dwarven.angle = 0.02F * time;
		angle = 0.00125F * time;
		radius = 200.0F;

		// Ideal radius = 600 => 200 => 600
		if(time < 12500)
			radius = 600.0F - (400.0F / 2500.0F) * (time - 10000);
		else if(time > 17500)
			radius = 600.0F - (400.0F / 2500.0F) * (20000 - time);

		for(i = 0; i < 7; i++) {
			glLoadIdentity();

			glTranslatef(radius * (float) cos((6.282F / 7 * i) + angle), radius * (float) sin((6.282F / 7 * i) + angle), -500.0F);
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, dwarven.colour);
			glRotatef(dwarven.angle, 1.0F, 1.0F, 0);
			draw_ring();
		}
	}

	// Nine for Mortal Men doomed to die
	else if(time < 30000) {
		mortals.angle = 0.02F * time;
		angle = 0.00025F * time;
		radius = 250.0F;

		// Ideal radius = 650 => 250 => 650
		if(time < 22500)
			radius = 650.0F - (400.0F / 2500.0F) * (time - 20000);
		else if(time > 27500)
			radius = 650.0F - (400.0F / 2500.0F) * (30000 - time);

		for(i = 0; i < 9; i++) {
			glLoadIdentity();

			glTranslatef(radius * (float) cos((6.282F / 9 * i) + angle), radius * (float) sin((6.282F / 9 * i) + angle), -600.0F);
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mortals.colour);
			glRotatef(mortals.angle, 1.0F, 1.0F, 0);
			draw_ring();
		}
	}

	// One for the Dark Lord on his dark throne
	else if(time < 65000) {
		sauron.angle = 0.05F * time + 35;

		glLoadIdentity();

		if(time - 30000 < 2500)
			glTranslatef(0, 0, -(float) (time - 30000) * 150.0F / 2500);
		else
			glTranslatef(0, 0, -150);

		// Post-texture specular colour requires OpenGL 1.2
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sauron.colour);
		glRotatef(sauron.angle, 1.0F, 1.0F, 0);

		draw_ring_textured();

		// Disable post texture specular again.
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	graphics_orthographic();
	text_colour (0.0,1.0,0.3);
	// Print a part of the ring lore verse.
	if(time >= 2000 && time < 6000) {
		opacity = fade_text(time - 2000);
		drift = drift_text(time - 2000);

		text_print(MID_X + drift, MID_Y - 25, "We started in 1991", opacity);
		text_print(MID_X - drift, MID_Y, "With TheParty...,", opacity);
	}

	else if(time >= 12000 && time < 16000) {
		opacity = fade_text(time - 12000);
		drift = drift_text(time - 12000);

		text_print(MID_X - drift, MID_Y - 25, "In 2006 we came", opacity);
		text_print(MID_X + drift, MID_Y, "to DreamHack,", opacity);
	}

	else if(time >= 22000 && time < 26000) {
		opacity = fade_text(time - 22000);
		drift = drift_text(time - 22000);

		text_print(MID_X + drift, MID_Y - 25, "We have been a", opacity);
		text_print(MID_X - drift, MID_Y, "Part of Dreamhack since!,", opacity);
	}

	else if(time >= 32000 && time < 36000) {
		opacity = fade_text(time - 32000);
		drift = drift_text(time - 32000);

		text_print(MID_X - drift, VIEW_SIZE_Y - 55, "Dreamhack...we love you!", opacity);
		text_print(MID_X + drift, VIEW_SIZE_Y - 30, "Lets mary with children,", opacity);
	}

	else if(time >= 37000 && time < 41000) {
		opacity = fade_text(time - 37000);
		drift = drift_text(time - 37000);

		text_print(MID_X + drift, VIEW_SIZE_Y - 55, "Ohhh.. wait.. we can't!", opacity);
		text_print(MID_X - drift, VIEW_SIZE_Y - 30, "too many love you all ready..", opacity);
	}

	else if(time >= 42000 && time < 46000) {
		opacity = fade_text(time - 42000);
		drift = drift_text(time - 42000);

		text_print(MID_X - drift, VIEW_SIZE_Y - 55, "Why do a DEMO?", opacity);
		text_print(MID_X + drift, VIEW_SIZE_Y - 30, "For the love of Dreamhack,", opacity);
	}

	else if(time >= 47000 && time < 51000) {
		opacity = fade_text(time - 47000);
		drift = drift_text(time - 47000);

		text_print(MID_X + drift, VIEW_SIZE_Y - 55, "Never seen Sweden?", opacity);
		text_print(MID_X - drift, VIEW_SIZE_Y - 30, "Or the lovely city?,", opacity);
	}

	else if(time >= 52000 && time < 56000) {
		opacity = fade_text(time - 52000);
		drift = drift_text(time - 52000);

		text_print(MID_X - drift, VIEW_SIZE_Y - 55, "Lets take a trip outside", opacity);
		text_print(MID_X + drift, VIEW_SIZE_Y - 30, "and into the woods.", opacity);
	}
}

void ring_poem_end(void)
{
	texture_free(1);
	texture_free(2);
}

void ring_poem_setup(void)
{
	generate_ring();

	// Dette er baggrundsbillederne
	if(!texture_load("data\\deadmeat-grup.bmp", 1))
		exit(1);

	// En Texture....
	if(!texture_load("data\\one_ring.bmp", 2))
		exit(1);
}