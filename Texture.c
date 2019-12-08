#include <windows.h>
#include <gl\glu.h>
#include <gl\gl.h>
#include <gl\glext.h>	// GL 1.2 Post texture specular.
#include <stdio.h>

#include "texture.h"

#pragma pack(1)

typedef struct
{
	unsigned short type;		// Must be "BM" for Windows bitmap.
	unsigned long file_size;	// Filesize in 32bit units.
	unsigned long reserved;		// Expansion?
	unsigned long data_offset;	// Offset of image data.
	unsigned long header_size;	// Size of the rest of the header.
	unsigned long size_x;		// Width in pixels.
	unsigned long size_y;		// Height in pixels.
	unsigned short planes;		// Bit planes in this image.
	unsigned short bpp;			// Colour depth of this image.
	unsigned long compressed;	// Compression method.
	unsigned long image_size;	// Size of the image in bytes.
	unsigned long xppm;			// Pixels per metre on x. Printing.
	unsigned long yppm;			// Pixels per metre on y. Printing.
	unsigned long colours_used;	// The number of colours used.
	unsigned long colours_need;	// Necessary number of colours.
} bmp_header_t;

typedef struct
{
	bmp_header_t h;				// Permits reading header in one go.
	unsigned char *data;		// Image pixel bits. Variable size.
} bmp_t;

#pragma pack()

// Texture mapping.
static unsigned int texture_index[64];

static int bitmap_import(bmp_t *b, const char *pathname)
{
	FILE *bmp_file;
	int i;

	if(!(bmp_file = fopen(pathname, "rb")))
		return 0;

	fread(&b->h, sizeof(bmp_header_t), 1, bmp_file);

	// Image dimensions must be multiples of 32.
	if(b->h.type != 0x4D42 || (b->h.size_x & 31) || (b->h.size_y & 31)
		|| b->h.planes != 1 || b->h.bpp != 24 || b->h.compressed) {
		fclose(bmp_file);
		return 0;
	}

	if(!(b->data = (unsigned char *) malloc(b->h.image_size))) {
		fclose(bmp_file);
		return 0;
	}

	// Bitmap images are stored upside down.
	// Scanlines are padded to align to 4 bytes. Not an issue here.
	for(i = (int) b->h.size_y - 1; i >= 0; i--)
		fread(&b->data[i * b->h.size_x * 3], sizeof(unsigned char), b->h.size_x * 3, bmp_file);

	fclose(bmp_file);

	return 1;
}

int texture_load(char *name, int index)
{
	bmp_t image;

	if(!bitmap_import(&image, name))
		return 0;

	glGenTextures(1, &texture_index[index]);
	glBindTexture(GL_TEXTURE_2D, texture_index[index]);

	// Are the mipmaps even used if we specify GL_LINEAR?
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image.h.size_x, image.h.size_y,
		GL_BGR, GL_UNSIGNED_BYTE, image.data);

	// Filtering modes for drawing textures.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Clean up what we allocated.
	free(image.data);

	return 1;
}

int texture_id(int i)
{
	return texture_index[i];
}

void texture_from_memory(int i, int size_x, int size_y, unsigned char *data)
{
	glGenTextures(1, &texture_index[i]);
	glBindTexture(GL_TEXTURE_2D, texture_index[i]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, size_x,
		size_y, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void texture_free(int index)
{
	unsigned int x;

	x = texture_id(index);

	glDeleteTextures(1, &x);
}
