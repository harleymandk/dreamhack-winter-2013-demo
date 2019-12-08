#include <windows.h>
#include <gl\glu.h>
#include <gl\gl.h>

#include "common.h"
#include "graphics.h"
#include "texture.h"
#include "text.h"

#define HEAD { 1.0F, 1.0F, 1.0F }
#define LINK { 0.5F, 0.5F, 0.5F }
#define MAIL { 0.5F, 0.5F, 0.5F }
#define NAME { 0.5F, 0.5F, 0.5F }

#define LINE_WIDTH 40
#define MAX_LINES 56

typedef struct {
	float c[3];
	char *line;
} script_t;

static script_t script[MAX_LINES] = {
	{ HEAD, "From DK With Love" },
	{ HEAD, "Done in C++ and stuff ;-)" },
	{ NAME, "as a part of the DreamHack Kreativ" },
	{ NAME, "Wild / Animation at" },
	{ NAME, "DreamHack Winter 2013" },

	{ HEAD, "Programming" },
	{ NAME, "Mr.Turner" },
	{ NAME, "HarleyMan" },
	{ NAME, "Heinstein" },

	{ HEAD, "Modelling" },
	{ NAME, "Mr.Turner" },
	{ NAME, "HarleyMan" },

	{ HEAD, "Textures and Skins" },
	{ NAME, "Mayang Murni Adnin" },
	{ LINK, "HarleyMan" },
	{ NAME, "SivSko" },
	{ NAME, "Heinstein" },

	{ HEAD, "Test,QA and Runner..." },
	{ NAME, "SivSko" },
	{ NAME, "The anomynous from DTU" },
	{ NAME, "Heinstein" },

	{ HEAD, "Forest Skybox Photo by:" },
	{ NAME, "SivSko" },
	{ NAME, "Heinstein" },
	{ HEAD, "Cut and fitted by:" },
	{ NAME, "Heinstein" },

	{ HEAD, "The Music" },
	{ NAME, "HAZEL-from Scenesat cd 1" },
	{ NAME, "From Denmark With Love" },

	{ HEAD, "Resources Used:" },
	{ NAME, "BASS from un4seen" },
	{ NAME, "Bitbucket (GIT repo)" },
	{ NAME, "MilkShape 3D with c/c++ from Bob Nemeth" },
	{ NAME, "MS Visual C++ 12.0" },
	{ NAME, "Ink Scape" },
	{ NAME, "Alot of coffee and Cult drinks" },

	{ HEAD, "Greetings to : " },
	{ NAME, "MIRROR" },
	{ NAME, "Loonies" },
	{ NAME, "Everybody from DTU@DHW13" },
	{ NAME, "and All Demo sceners at DHW13" },
	{ NAME, "IRC : #dreamhack.kreativ" },

	{ HEAD, "You Have been Watching a PC-demo from" },
	{ NAME, "The DeadMeat community." },
	{ NAME, "We named it:From DK With Love" },
	{ NAME, "Link to this demo and" },
	{ NAME, "info about our community"},
	{ LINK, "at http://www.deadmeat.dk" },

	{ LINK, "Thanks for watching!" },

	{ HEAD, "We are Out......click!  press ESC to exit!" }

};

void credits_process(unsigned long time)
{
	int i, j, k, offset;
	float pan;

	pan = time * 0.04F;

	glClear(GL_COLOR_BUFFER_BIT);
	glShadeModel(GL_FLAT);
	glEnable(GL_TEXTURE_2D);

	// While there is still text on the screen.
	if(pan + VIEW_SIZE_Y > 500 && pan < MAX_LINES * LINE_WIDTH + 500) {
		i = ((int) (pan) - 500) / LINE_WIDTH;
		j = i + VIEW_SIZE_Y / LINE_WIDTH;
		offset = (int) pan;
		offset -= 500;
		offset %= LINE_WIDTH;
		offset = -offset;
		k = 0;

		if(i < 0) {
			offset = 500 - (int) (pan);
			i = 0;
		}

		else j++;

		if(j >= MAX_LINES)
			j = MAX_LINES - 1;

		while(i < j) {
			text_colour(script[i].c[0], script[i].c[1], script[i].c[2]);
			text_print(VIEW_SIZE_X >> 1, (float) (offset + (k * LINE_WIDTH)), script[i].line, 1);
			k++;
			i++;
		}
	}

	text_colour(1, 1, 1);
	glDisable(GL_TEXTURE_2D);
}

void credits_end(void)
{
	// Free the font at last.
	texture_free(0);
}

void credits_setup(void)
{
	// Set openGL Attributes just once.
	graphics_orthographic();
}