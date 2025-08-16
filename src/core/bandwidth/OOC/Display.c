/*============================================================================
  Display, an object-oriented C OpenGL display class.
  Copyright (C) 2019, 2024 by Zack T Smith.

  Object-Oriented C is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  Object-Oriented C is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public License
  along with this software.  If not, see <http://www.gnu.org/licenses/>.

  The author may be reached at 1@zsmith.co.
 *===========================================================================*/

// This code is derived from my FrugalWidgets project.

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <sys/types.h>
#include <dirent.h>

#include "Keycodes.h"
#include "DateTime.h"
#include "Display.h"
#include "Window.h"
#include "Log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <unistd.h> // usleep
#include <time.h> // time

#if defined(HAVE_OPENGL) && defined(HAVE_GLUT)
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glut.h>
#endif

static unsigned nDisplays = 0;
static Display *displays[MAX_DISPLAYS];

DisplayClass *_DisplayClass = NULL;

void Display_destroy (Any *self_)
{
	DEBUG_DESTROY;
	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_,Display);
}
	
unsigned Display_width (Display* self)
{
	if (!self)
		return 0;
	verifyCorrectClassOrSubclass(self,Display);
	return self->width;
}

unsigned Display_height (Display* self)
{
	if (!self)
		return 0;
	verifyCorrectClassOrSubclass(self,Display);
	return self->height;
}

unsigned Display_flags (Display* self)
{
	if (!self)
		return 0;
	verifyCorrectClassOrSubclass(self,Display);
	return self->flags;
}

void Display_setFlags (Display* self, unsigned value)
{
	if (!self)
		return;
	verifyCorrectClassOrSubclass(self,Display);
	self->flags = value;
}

void Display_describe (Display* self, FILE *file)
{
	if (!self)
		return;
	verifyCorrectClassOrSubclass(self,Display);
	
	fprintf (file ?: stdout, "%s\n", $(self, className));
}

void Display_addWindow (Display *self, Any *window)
{
	if (!self || !window) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Display);
	verifyCorrectClassOrSubclass(window,Window);

	if (self->nWindows < MAX_WINDOWS_PER_DISPLAY) {
		self->windows[self->nWindows++] = window;
	}
}

Any* Display_lookupWindowByID (Display *self, long soughtIdentifier)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Display);

	if (soughtIdentifier == INVALID_WINDOW_ID) {
		return NULL;
	}

	for (unsigned short i=0; i < self->nWindows; i++) {
		Window *window = self->windows[i];
		long foundIdentifier = $(window, identifier);
		if (foundIdentifier == soughtIdentifier) {
			return window;
		}
	}

	return NULL;
}

const char *Display_name (Display *self)
{
	if (!self)
		return NULL;
	verifyCorrectClass(self,Display);

	return "None";
}

#if defined(HAVE_GLUT)
static bool lookup_display_and_window (Display **return_dpy, Window **return_win)
{
	int identifier = glutGetWindow();
	CHECK_GL_ERROR("glutGetWindow");
	for (unsigned i = 0; i < nDisplays; i++) {
		Display *display = displays[i];
		Window *window = Display_lookupWindowByID (display, identifier);
		if (window) {
			*return_dpy = display;
			*return_win = window;
			return true;
		}
	}
	*return_dpy = NULL;
	*return_win = NULL;
	return false;
}

//---------------------------------------------------------------------------
// Name:	draw_scene
// Purpose:	Draws the entire window contents.
//---------------------------------------------------------------------------
static time_t last_time = 0;
static unsigned framesPerSecond = 0;
static uint64_t frameStartTime = 0;

static void draw_scene ()
{
	frameStartTime = DateTime_getMicrosecondTime();

	Display *display;
	Window *window;
	if (!lookup_display_and_window (&display, &window)) {
		Log_warning (__FUNCTION__, "Lookup by display & window failed.");
		return;
	}
	//==========

	if ($(window, identifier) < 0) {
		return;
	}

	$(window, redraw);
	glutPostRedisplay ();

	time_t now = time(NULL);
	if (now != last_time) {
		if (framesPerSecond && last_time) {
			static char message[64];
			snprintf (message, sizeof(message), "Frames per Second = %u", framesPerSecond);
			$(window, setTopLeftMessage, message);
		}
		last_time = now;
		framesPerSecond = 0;
	}
	framesPerSecond++;
}

static void handle_keypress (unsigned char asciiKey, int x, int y)
{
	Display *display;
	Window *window;
	if (!lookup_display_and_window (&display, &window)) {
		Log_warning (__FUNCTION__, "Lookup by display & window failed.");
		return;
	}
	//==========

	if (asciiKey >= 32 && asciiKey < 127) {
		Log_debug_printf (__FUNCTION__, "Key pressed %c (0x%02x)", asciiKey, asciiKey);
	} else {
		Log_debug_printf (__FUNCTION__, "Key pressed 0x%02x", asciiKey);
	}

	if (asciiKey == 27) {
		$(window, message, kQuittingNowMessage, NULL, 0, 0);
		exit(0);
	} else {
		$(window, message, kKeyDownMessage, NULL, asciiKey, 0);
		$(window, message, kKeyUpMessage, NULL, asciiKey, 0);
	}
}

static void handle_resize (int w, int h) 
{
	Display *display;
	Window *window;
	if (!lookup_display_and_window (&display, &window)) {
		Log_warning (__FUNCTION__, "Lookup by display & window failed.");
		return;
	}
	//==========

	$(window, resize, w, h);
}

static void handle_idle () 
{
	uint64_t us = DateTime_getMicrosecondTime ();
	uint64_t maximumFPS = 120;
	uint64_t usPerFrame = 1000000 / maximumFPS;
	uint64_t elapsed = us - frameStartTime;
	if (elapsed < usPerFrame) {
		uint64_t delta = usPerFrame - elapsed;
		usleep (delta);
	}
}

static void handle_mouse (int button, int state, int x, int y) 
{
	Display *display;
	Window *window;
	if (!lookup_display_and_window (&display, &window)) {
		Log_warning (__FUNCTION__, "Lookup by display & window failed.");
		return;
	}
	//==========

	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN)
			$(window, message, kPointerDownMessage, NULL, x, y);
		else
			$(window, message, kPointerUpMessage, NULL, x, y);
	} else {
		Log_debug (__FUNCTION__, "OTHER MOUSE BUTTON PRESSED.");
	}
}

static void handle_motion (int x, int y) 
{
	Display *display;
	Window *window;
	if (!lookup_display_and_window (&display, &window)) {
		Log_warning (__FUNCTION__, "Lookup by display & window failed.");
		return;
	}
	//==========

	if (window) {
		$(window, message, kPointerMovedMessage, NULL, x, y);
	} 
}

static void handle_special (int incoming_key, int mouse_x, int mouse_y) 
{
	Display *display;
	Window *window;
	if (!lookup_display_and_window (&display, &window)) {
		Log_warning (__FUNCTION__, "Lookup by display & window failed.");
		return;
	}
	//==========

	int key = 0;
	switch (incoming_key) {
		case GLUT_KEY_LEFT:	key = kKeycode_Left; break;
		case GLUT_KEY_RIGHT:	key = kKeycode_Right; break;
		case GLUT_KEY_UP:	key = kKeycode_Up; break;
		case GLUT_KEY_DOWN:	key = kKeycode_Down; break;
		case GLUT_KEY_PAGE_UP:	key = kKeycode_PageUp; break;
		case GLUT_KEY_PAGE_DOWN:key = kKeycode_PageDown; break;
		case GLUT_KEY_HOME:	key = kKeycode_Home; break;
		case GLUT_KEY_END:	key = kKeycode_End; break;
		case GLUT_KEY_F1:	key = kKeycode_F1; break;
		case GLUT_KEY_F2:	key = kKeycode_F2; break;
		case GLUT_KEY_F3:	key = kKeycode_F3; break;
		case GLUT_KEY_F4:	key = kKeycode_F4; break;
		case GLUT_KEY_F5:	key = kKeycode_F5; break;
		case GLUT_KEY_F6:	key = kKeycode_F6; break;
		case GLUT_KEY_F7:	key = kKeycode_F7; break;
		case GLUT_KEY_F8:	key = kKeycode_F8; break;
		case GLUT_KEY_F9:	key = kKeycode_F9; break;
		case GLUT_KEY_F10:	key = kKeycode_F10; break;
		case GLUT_KEY_F11:	key = kKeycode_F11; break;
		case GLUT_KEY_F12:	key = kKeycode_F12; break;
		case GLUT_KEY_INSERT:	key = kKeycode_Insert; break;

		default:
			printf("Other key %d\n", incoming_key);
	}

	if (key) {
		$(window, message, kKeyDownMessage, NULL, key, 0);
		$(window, message, kKeyUpMessage, NULL, key, 0);
	}
}

static void handle_visibility (int value)
{
	Display *display;
	Window *window;
	if (!lookup_display_and_window (&display, &window)) {
		Log_warning (__FUNCTION__, "Lookup by display & window failed.");
		return;
	}
	//==========
	$(window, message, kWindowVisibilityMessage, NULL, value, 0);
}

static void handle_passivemotion (int x, int y)
{
	// Mouse motion when button not pressed.
}

static void handle_entry (int value)
{
	Display *display;
	Window *window;
	if (!lookup_display_and_window (&display, &window)) {
		Log_warning (__FUNCTION__, "Lookup by display & window failed.");
		return;
	}
	//==========
	$(window, message, kWindowPointerEntryMessage, NULL, value, 0);
}

#endif

unsigned Display_brightness (Display *self)
{
#if defined(__linux__) 
	// Read /sys/class/backlight/acpi_video0/brightness
	DIR *dir = opendir ("/sys/class/backlight");
	struct dirent *entry;
	while ((entry = readdir (dir))) {
		// TODO
		;
	}
	closedir (dir);
#endif
	return 0;
}

Display* Display_init (Display* self)
{
	ENSURE_CLASS_READY(Display);

	if (nDisplays >= MAX_DISPLAYS) {
		release (self);
		return NULL;
	}

	if (self) {
		Object_init ((Object*)self);
		self->is_a = _DisplayClass;

		self->nWindows = 0;

#if defined(HAVE_GLUT)
		int argc = 0;
		static const char *argv[] = { 
			"abc123" 
		};
		glutInit (&argc, (char**) argv);
		glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
#endif

		Object_init ((Object*)self);
		self->is_a = _DisplayClass;
#if defined(HAVE_GLUT)
		self->width = glutGet (GLUT_SCREEN_WIDTH);
		self->height = glutGet (GLUT_SCREEN_HEIGHT);
		self->depth = 32;
#endif

        	Log_debug_printf (__FUNCTION__, "Display size=%ux%u", self->width, self->height);
	}

	displays[nDisplays++] = self;

	return self;
}

static bool Display_canAddWindows (Display *self)
{
	if (!self) {
		return false;
	}
	verifyCorrectClass(self,Display);
	return self->nWindows < MAX_WINDOWS_PER_DISPLAY;
}

void Display_mainLoop (Display *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Display);

#if defined(HAVE_GLUT)
        glutDisplayFunc (&draw_scene);
        glutKeyboardFunc (&handle_keypress);
        glutSpecialFunc (&handle_special);
        glutReshapeFunc (&handle_resize);
        glutIdleFunc (&handle_idle);
        glutMouseFunc (&handle_mouse);
        glutMotionFunc (&handle_motion);
	glutVisibilityFunc (&handle_visibility);
	glutPassiveMotionFunc (&handle_passivemotion);
	glutEntryFunc (&handle_entry);
        glutMainLoop ();
#endif
}

DisplayClass* DisplayClass_init (DisplayClass *class)
{
	SET_SUPERCLASS(Object);

	SET_INHERITED_METHOD_POINTER(Display,Object,print);
	SET_INHERITED_METHOD_POINTER(Display,Object,equals);

	SET_OVERRIDDEN_METHOD_POINTER(Display,describe);

        SET_METHOD_POINTER(Display,width);
        SET_METHOD_POINTER(Display,height);
	SET_METHOD_POINTER(Display,name);
	SET_METHOD_POINTER(Display,flags);
	SET_METHOD_POINTER(Display,setFlags);
	SET_METHOD_POINTER(Display,addWindow);
	SET_METHOD_POINTER(Display,brightness);
	SET_METHOD_POINTER(Display,lookupWindowByID);
	SET_METHOD_POINTER(Display,mainLoop);
	SET_METHOD_POINTER(Display,canAddWindows);

	VALIDATE_CLASS_STRUCT(class);
	return class;
}

