/*
Start på Demo. Og musik delen..  Lavede ud fra BASS fra Un4seen.
ToDO:  
* pænere splash logo.... hvad skal der stå???
* fix Skybox  - Passer ikke sammen... google?
* Musik. Skal den ind i 64K skal musikken fylde mindre...  (BASS kan spille : xm , IT og mod filer..IKKE ogg/mp3!!)
* Noget andet end ringe...   kode/ideer modtages ;-)
* Afsnit med Shaders? (how to do???)
* musik kunne pakkes med : Clinkster eller noget bin2c ????
-----------------------------------
For at bruge denne demo skal du have :
Visual Studio v10 eller højere med C/c++ 
OpenGL
GLEW (The OpenGL Extension Wrangler Library )
BASS.dll  (musik) 

WinAmp til hører .xm musik ;-)
EXE filen pakkes med :  kkrunchy.exe "From Dk With Love.exe"
*/

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "bass.lib")

#include <windows.h>

#include "common.h"
#include "process.h"
#include "bass.h"

// Windows details.
#define WINDOW_CLASS "From DK with Love"
#define WINDOW_ERROR "Ohh Crap! Terminal error"
#define WINDOW_NAME "From DK with Love"

// Window style attributes.
#define FRAME_STYLE_REG (((WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) \
	| WS_CLIPCHILDREN | WS_CLIPSIBLINGS) ^ (WS_MAXIMIZEBOX | WS_MINIMIZEBOX))

#define FRAME_STYLE_EXT (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE)

static HSTREAM music;

static long __stdcall frame_process(HWND window, unsigned int message, unsigned int wparam, long lparam)
{
	switch(message) {
		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(window, message, wparam, lparam);
	}

	return 0;
}

static HWND frame_create(HINSTANCE app_instance, int full_screen)
{
	DEVMODE graphics_mode;
	RECT client_region;
	WNDCLASS wc;
	HWND result;

	// Fill in the window class structure.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hIcon = LoadIcon(app_instance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(app_instance, IDC_ARROW);
	wc.lpszClassName = WINDOW_CLASS;
	wc.lpfnWndProc = frame_process;
	wc.hbrBackground = 0;
	wc.lpszMenuName = 0;
	wc.hInstance = app_instance;
	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;

	if(!RegisterClass(&wc))
		return 0;

	if(full_screen) {
		// Switch to full screen mode.
		memset(&graphics_mode, 0, sizeof(graphics_mode));

		graphics_mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		graphics_mode.dmSize = sizeof(graphics_mode);
		graphics_mode.dmPelsHeight = VIEW_SIZE_Y;
		graphics_mode.dmPelsWidth = VIEW_SIZE_X;
		graphics_mode.dmBitsPerPel = VIEW_BPP;

		if(ChangeDisplaySettings(&graphics_mode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			UnregisterClass(WINDOW_CLASS, app_instance);

			return 0;
		}
	}

	// Specify client region dimensions.
	client_region.bottom = VIEW_SIZE_Y;
	client_region.right = VIEW_SIZE_X;
	client_region.top = 0;
	client_region.left = 0;

	// Ask windows to work out how big the window must be.
	AdjustWindowRectEx(&client_region, full_screen ? WS_POPUP : FRAME_STYLE_REG,
		0, FRAME_STYLE_EXT);

	// Create a window using this window class and style.
	result = CreateWindowEx(FRAME_STYLE_EXT, WINDOW_CLASS, WINDOW_NAME,
		full_screen ? WS_POPUP : FRAME_STYLE_REG, 0, 0, client_region.right
		- client_region.left, client_region.bottom - client_region.top, NULL, NULL,
		app_instance, NULL);

	if(!result) {
		// Destroy the window class we created.
		UnregisterClass(WINDOW_CLASS, app_instance);
	}

	return result;
}

static void frame_destroy(HINSTANCE app_instance, HWND frame, HDC context)
{
	ReleaseDC(frame, context);
	DestroyWindow(frame);
	UnregisterClass(WINDOW_CLASS, app_instance);
}

static void frame_decouple_gl(HGLRC context)
{
	if(!wglMakeCurrent(0, 0))
		MessageBox(0, "Error releasing Open GL render context.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

	if(!wglDeleteContext(context))
		MessageBox(0, "Open GL context could not be deleted.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);
}

static HGLRC frame_assign_gl(HWND window, HDC context)
{
	PIXELFORMATDESCRIPTOR pixel_format = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL
		| PFD_DOUBLEBUFFER | PFD_TYPE_RGBA,
		VIEW_BPP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 16, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	HGLRC frame_render;
	unsigned int frame_format;

	if(!(frame_format = ChoosePixelFormat(context, &pixel_format))) {
		MessageBox(0, "A suitible pixel format was not found,",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		return 0;
	}

	if(!SetPixelFormat(context, frame_format, &pixel_format)) {
		MessageBox(0, "Pixel format setting failed on supported mode.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		return 0;
	}

	// Er der OpenGL ?
	if(!(frame_render = wglCreateContext(context))) {
		MessageBox(0, "Open GL render context was not created.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		return 0;
	}

	if(!wglMakeCurrent(context, frame_render)) {
		MessageBox(0, "Open GL render context activation failed.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		frame_decouple_gl(frame_render);

		return 0;
	}

	return frame_render;
}

static int music_start(HWND handle, char *pathname)
{
	if(BASS_GetVersion() != MAKELONG(1, 8)) {
		MessageBox(0, "Incorrect BASS version. Expected v1.8.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		return 0;
	}

	if(!BASS_Init(-1, 44100, 0, handle)) {
		MessageBox(0, "Fail to initialize the sound system.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		return 0;
	}

	BASS_Start();
	BASS_StreamFree(music);

	if(!(music = BASS_StreamCreateFile(FALSE, pathname, 0, 0, 0))) {
		MessageBox(0, "Error creating file stream from disk.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		return 0;
	}

	if(!BASS_StreamPlay(music, FALSE, 0)) {
		MessageBox(0, "Error playing file stream.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		return 0;
	}

	return 1;
}

static void music_stop(void)
{
	BASS_ChannelStop(music);
	BASS_Free();
}

int __stdcall WinMain(HINSTANCE current, HINSTANCE prior, char *argument, int visible)
{
	HGLRC frame_render;
	HWND frame_handle;
	HDC frame_device;
	MSG message;
	int full_screen;
	int finished;

	// Full screen defaults to off.
	full_screen = 0;

#ifndef BUILD_DEBUG
	if(MessageBox(0, "Would you like to run in full screen mode?",
		WINDOW_NAME, MB_ICONQUESTION | MB_YESNO) == IDYES)
		full_screen = 1;
#endif

	if(!(frame_handle = frame_create(current, full_screen))) {
		MessageBox(0, "Unable to create a frame window.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		return 0;
	}

	// Get a handle to the windows device context.
	frame_device = GetDC(frame_handle);

	if(!(frame_render = frame_assign_gl(frame_handle, frame_device))) {
		MessageBox(0, "Unable to assign Open GL to the window.",
			WINDOW_ERROR, MB_ICONERROR | MB_OK);

		frame_destroy(current, frame_handle, frame_device);

		return 0;
	}

#ifndef BUILD_DEBUG
	// Start the music playing.
 if(!music_start(frame_handle, "data\\music.mp3")) {
		frame_destroy(current, frame_handle, frame_device);

		return 0;
	}
#endif

	// Start the window rolling.
	ShowWindow(frame_handle, visible);
	SetForegroundWindow(frame_handle);
	SetFocus(frame_handle);

	if(full_screen) {
		// Full screen mode has no cursor.
		ShowCursor(FALSE);
	}

	do {
		finished = 0;

		if(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			// We need to process this message.
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		else {
			// No messages, so we process game code.
			finished = process_control();
			SwapBuffers(frame_device);
		}
	} while(message.message != WM_QUIT && !finished);

#ifndef BUILD_DEBUG
	// Stop the music.
	music_stop();
#endif

	if(full_screen) {
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
	}

	frame_decouple_gl(frame_render);
	frame_destroy(current, frame_handle, frame_device);

	return message.wParam;
}