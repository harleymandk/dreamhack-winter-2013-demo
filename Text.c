/*
Proportionally spaced text. The code relies on
an 8bpp Alpha layer that represents the text. This
is read from a file that also holds the widths
of each character glyph. Using vertex colours,
this text can be displayed in any colour (see
credits.c for examples). It can also be faded in
and out.

The first scene in the demo loads up the font. This
font isn't freed until the last scene in the demo
has been shown.
*/

#include <windows.h>
#include <gl\glu.h>
#include <gl\gl.h>
#include <stdio.h>

#include "texture.h"
#include "text.h"

#pragma pack(1)

typedef struct
{
	unsigned short image_x;		// Meets OpenGL texture size restraints.
	unsigned short image_y;		// Meets OpenGL texture size restraints.
	unsigned short glyph_x;		// Less than 256. 16 bits for pixel data alignment.
	unsigned short glyph_y;		// Less than 256. 16 bits for pixel data alignment.
	unsigned char *data;		// Pixel data. Aligned to four byte boundry.
	unsigned char *width;		// Glyph width information for proportional spacing.
} font_t;

#pragma pack()

static font_t f;
static float colour[3] = { 1, 1, 1 };
static int font_texture;

static int text_string_length(char *data)
{
	int length;

	length = 0;

	while(*data) {
		if(*data == ' ')
			length += 12;
		else
			length += f.width[(*data) - '!'] + 2;

		data++;
	}

	return length;
}

void text_print(float x, float y, char *data, float opacity)
{
	float u, v;
	int glyph_x, glyph_y;
	int width, glyph;

	if(!data)
		return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texture_id(font_texture));
	glColor4f(colour[0], colour[1], colour[2], opacity);

	// Default to centred text at the moment.
	x -= text_string_length(data) >> 1;
	u = 1.0F / f.image_x;
	v = 1.0F / f.image_y;

	while(*data) {
		width = 12;

		if(*data != ' ') {
			glyph = (*data) - '!';
			glyph_x = (glyph % (f.image_x / f.glyph_x)) * f.glyph_x;
			glyph_y = (glyph / (f.image_x / f.glyph_x)) * f.glyph_y;
			width = f.width[glyph] + 2;

			// Draw the glyph onto a textured quad.
			glBegin(GL_QUADS);
			glTexCoord2f(u * glyph_x, v * glyph_y);
			glVertex2f(x, y);
			glTexCoord2f(u * (glyph_x + width), v * glyph_y);
			glVertex2f(x + width, y);
			glTexCoord2f(u * (glyph_x + width), v * (glyph_y + f.glyph_y));
			glVertex2f(x + width, y + f.glyph_y);
			glTexCoord2f(u * glyph_x, v * (glyph_y + f.glyph_y));
			glVertex2f(x, y + f.glyph_y);
			glEnd();
		}

		x += width;
		data++;
	}

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void text_colour(float r, float g, float b)
{
	colour[0] = r;
	colour[1] = g;
	colour[2] = b;
}

int text_font(char *pathname, int index)
{
	FILE *font_file;
	int glyphs;

	if(!(font_file = fopen(pathname, "rb")))
		return 0;

	fread(&f.image_x, sizeof(unsigned short), 4, font_file);

	glyphs = (f.image_x / f.glyph_x) * (f.image_y / f.glyph_y);

	if(!(f.data = (unsigned char *) malloc(f.image_x * f.image_y))) {
		fclose(font_file);
		return 0;
	}

	if(!(f.width = (unsigned char *) malloc(glyphs))) {
		fclose(font_file);
		free(f.data);
		return 0;
	}

	fread(f.data, sizeof(unsigned char), (int) f.image_x * f.image_y, font_file);
	fread(f.width, sizeof(unsigned char), glyphs, font_file);
	fclose(font_file);

	texture_from_memory(index, f.image_x, f.image_y, f.data);
	free(f.data);

	font_texture = index;
	f.data = 0;

	return 1;
}