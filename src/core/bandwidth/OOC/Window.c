/*============================================================================
  Window, an object-oriented C window class.
  Copyright (C) 2019, 2022, 2024 by Zack T Smith.

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

#include "Window.h"
#include "Controller.h"
#include "View.h"
#include "Log.h"

#include <string.h>

#ifdef HAVE_OPENGL
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glut.h>

static GLint lightNames[MAX_LIGHTS] = {
	GL_LIGHT0,
	GL_LIGHT1,
	GL_LIGHT2,
	GL_LIGHT3,
	GL_LIGHT4,
	GL_LIGHT5,
	GL_LIGHT6,
	GL_LIGHT7,
};
#endif

WindowClass *_WindowClass = NULL;

Window* Window_init (Window *self)
{
	ENSURE_CLASS_READY(Window);

	if (self) {
		Object_init ((Object*) self);
		self->is_a = _WindowClass;
		self->identifier = INVALID_WINDOW_ID;
		self->modelsArray = retain(new(MutableArray));
		self->viewsArray = retain(new(MutableArray));
		self->cameraVector.x = 0.f;
		self->cameraVector.y = 0.f;
		self->cameraVector.z = 2.f;
		self->focusVector.x = 0.f;
		self->focusVector.y = 0.f;
		self->focusVector.z = 0.f;
		self->upVector.x = 0.f;
		self->upVector.y = 1.f;
		self->upVector.z = 0.f;

		self->useLight[0] = true;
		self->lightAmbientColors[0] = 0x808080;
		self->lightDiffuseColors[0] = RGB_WHITE;
		self->lightSpecularColors[0] = RGB_WHITE;
		self->lightPositions[0] = Vector_new(0,0,0);
	}

	return self;
}

void Window_destroy (Any *self_)
{
        DEBUG_DESTROY;

	if (!self_) {
		return;
	}
	verifyCorrectClass(self_,Window);

	Window *self = self_;

	if (self->identifier >= 0) {
#ifdef HAVE_OPENGL
		glutDestroyWindow (self->identifier);
#endif
	}

	Object_destroy (self);
}

void Window_print (Window* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Window);

	if (!outputFile) {
		outputFile = stdout;
	}

	fprintf (outputFile, "%s(#%ld %ux%u %d,%d; title \"%s\")", $(self, className), self->identifier, 
		self->width, 
		self->height, 
		self->x, 
		self->y,
		self->title);
}

void Window_describe (Window* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Window);

	if (!outputFile) {
		outputFile = stdout;
	}

	fprintf (outputFile, "%s", $(self, className));
}

static long Window_identifier (Window* self)
{ 
	if (!self) {
		return INVALID_WINDOW_ID;
	}
	verifyCorrectClass(self,Window);
	return self->identifier;
}

static void Window_setIdentifier (Window* self, long identifier)
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Window);
	self->identifier = identifier;
}

static void Window_move (Window *self, int x, int y)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Window);

	self->needsRedraw = true;
	self->x = x;
	self->y = y;
#ifdef HAVE_OPENGL
	glutSetWindow (self->identifier);
	glutPositionWindow (x, y);
#endif
}

static void Window_resize (Window* self, unsigned width, unsigned height)
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Window);

	if (!width || !height) {
		return;
	}

	self->needsRedraw = true;
	self->width = width;
	self->height = height;

	if (self->controller) {
		$(((Controller*)self->controller), layout);
	}
}

#ifdef HAVE_OPENGL
static void Window_initGLForModel (Window *self)
{
	int width = self->width;
	int height = self->height;

	glViewport (0, 0, width, height);
	glMatrixMode (GL_PROJECTION); 
	glLoadIdentity (); 
	double minZPlane = 0.01;
	double maxZPlane = 1000.0;
	double aspectRatio = (float) width / (float) height;
	gluPerspective (45.0f, aspectRatio, minZPlane, maxZPlane);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	gluLookAt (self->cameraVector.x,
		self->cameraVector.y,
		self->cameraVector.z,
		self->focusVector.x,
		self->focusVector.y,
		self->focusVector.z,
		self->upVector.x,
		self->upVector.y,
		self->upVector.z);
}
#endif

static void Window_moveCamera (Window* self, Vector vector)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Window);

	self->cameraVector.x = vector.x;
	self->cameraVector.y = vector.y;
	self->cameraVector.z = vector.z;
}

static unsigned Window_width (Window *self)
{
	if (!self)
		return 0;
	verifyCorrectClass(self,Window);

	return self->width;
}

static unsigned Window_height (Window *self)
{
	if (!self)
		return 0;
	verifyCorrectClass(self,Window);

	return self->height;
}

Rect Window_redraw (Window *self)
{
	Rect zeroRect = Rect_zero();

	if (!self || self->identifier < 0) { 
		return zeroRect;
	}
	verifyCorrectClass(self,Window);

	if (self->controller) {
		Controller *controller = self->controller;
		$(controller, layout); // Layout 2D widgets, if applicable.
		$(controller, update); // Revise 3D scene, if applicable.
	}

#ifdef HAVE_OPENGL
	long width = self->width;
	long height = self->height;

	glutSetWindow (self->identifier);

	Window_initGLForModel (self);

	glEnable (GL_NORMALIZE);
	glEnable (GL_COLOR_MATERIAL);
	glEnable (GL_LIGHTING);
	glEnable (GL_DEPTH_TEST);
	glLightModeli (GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	float overallAmbient[] = { 0.15, 0.15, 0.15, 1 };
	glLightModelfv (GL_LIGHT_MODEL_AMBIENT, overallAmbient);

	RGBA background_red = (self->backgroundColor >> 16) & 0xff;
	RGBA background_green = (self->backgroundColor >> 8) & 0xff;
	RGBA background_blue = self->backgroundColor & 0xff;
	glClearColor (background_red / 255.f, background_green / 255.f, background_blue / 255.f, 1);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat zero[] = { 0, 0, 0, 0 };

	for (int i=0; i < MAX_LIGHTS; i++) {
		float values[4];
		RGBA color = self->lightDiffuseColors[i];
		values[0] = ((color >> 16) & 0xff) / 255.f;
		values[1] = ((color >> 8) & 0xff) / 255.f;
		values[2] = (color & 0xff) / 255.f;
		values[3] = 1.f - (((color >> 24) & 0xff) / 255.f);
		glLightfv (lightNames[i], GL_DIFFUSE, values); 

		color = self->lightAmbientColors[i];
		values[0] = ((color >> 16) & 0xff) / 255.f;
		values[1] = ((color >> 8) & 0xff) / 255.f;
		values[2] = (color & 0xff) / 255.f;
		values[3] = 1.f - (((color >> 24) & 0xff) / 255.f);
		glLightfv (lightNames[i], GL_AMBIENT, values); 

		if (self->useLight[i]) {
			glEnable (lightNames[i]);
		} else {
			glDisable (lightNames[i]);
		}
		
		glPushMatrix();
		glTranslated(self->lightPositions[i].x, self->lightPositions[i].y, self->lightPositions[i].z);
		glLightfv(GL_LIGHT1, GL_POSITION, zero);
		glPopMatrix();
	}

	unsigned nModels = $(self->modelsArray, count);
	if (nModels) {
		glViewport (0, 0, width, height); 

		glPushMatrix ();
		for (unsigned i=0; i < nModels; i++) {
			Model3D *model = $(self->modelsArray, at, i);
			RGBA color = model->color;

			for (unsigned j=0; j < model->nTriangles; j++) {
				if (model->color == RGB_TEST_PATTERN) {
					color = j & 1 ? RGB_GREEN: RGB_RED;
				}

				RGBA red = (color >> 16) & 0xff;
				RGBA green = (color >> 8) & 0xff;
				RGBA blue = color & 0xff;
				GLfloat colorvalues[] = { red / 255.f, green / 255.f, blue / 255.f, 1.f};
				glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorvalues); 
				glMaterialfv (GL_FRONT, GL_EMISSION, zero);

				glBegin (GL_TRIANGLES);
				Triangle triangle = model->triangles[j];
				glNormal3f (triangle.normal_a.x, triangle.normal_a.y, triangle.normal_a.z);
				glVertex3f (triangle.a.x, triangle.a.y, triangle.a.z);
				glNormal3f (triangle.normal_b.x, triangle.normal_b.y, triangle.normal_b.z);
				glVertex3f (triangle.b.x, triangle.b.y, triangle.b.z);
				glNormal3f (triangle.normal_c.x, triangle.normal_c.y, triangle.normal_c.z);
				glVertex3f (triangle.c.x, triangle.c.y, triangle.c.z);
				glEnd ();
			}
		}
		glPopMatrix ();
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable (GL_TEXTURE_2D);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, width, 0, height, -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	unsigned nViews = $(self->viewsArray, count);
	if (nViews) {
		glPushMatrix ();

		for (unsigned i=0; i < nViews; i++) {
			View *view = $(self->viewsArray, at, i);
			if ($(view, hidden)) {
				continue;
			}
			$(view, redraw);
			unsigned textureID = view->textureID;
			RGBA *pixels = (RGBA*) $(view->image, pixels);
			if (!pixels || textureID == INVALID_TEXTURE_ID) {
				continue;
			}

			GLint x = view->rect.origin.x;
			GLint y = view->rect.origin.y;
			GLint w = view->rect.size.width;
			GLint h = view->rect.size.height;

			glBindTexture (GL_TEXTURE_2D, textureID);
			CHECK_GL_ERROR("glBindTexture");

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);

			glBegin (GL_QUADS);

			float invertedY = height - h - y;

			glNormal3f (0,0,1);
			glTexCoord2d (0.0, 1.0); 
			glVertex3d (x, invertedY, 0);
			
			glNormal3f (0,0,1);
			glTexCoord2d (1.0, 1.0); 
			glVertex3d (x+w, invertedY, 0);
			
			glNormal3f (0,0,1);
			glTexCoord2d (1.0, 0.0); 
			glVertex3d (x+w, invertedY+h, 0);
			
			glNormal3f (0,0,1);
			glTexCoord2d (0.0, 0.0);
			glVertex3d (x, invertedY+h, 0);
			glEnd ();
		}

		glPopMatrix ();

		glBindTexture (GL_TEXTURE_2D, 0);
		CHECK_GL_ERROR("glBindTexture ");
	}

	char *message = (char*) self->topLeftMessage;
	if (message) {
		glDisable (GL_LIGHTING);
		glViewport (0, 0, width, height); 
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, width, 0, height, -1, 1);
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		glColor3f (1, 1, 1);
		glRasterPos2f(5,height-20);
		while (*message) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *message);
			message++;
		}
	}

	glFlush ();
	CHECK_GL_ERROR("glFlush");

	glutSwapBuffers ();
	CHECK_GL_ERROR("glutSwapBuffers");
#endif

	self->needsRedraw = false;

	return zeroRect;
}

void Window_selfWasShown (Window* window) {
	puts(__FUNCTION__);
}

void Window_selfWasHidden (Window* window) {
	puts(__FUNCTION__);
}

void Window_didResize (Window* self) {
	puts(__FUNCTION__);
}

void Window_pointerDidEnter (Window* self) {
	puts(__FUNCTION__);
}

void Window_pointerDidLeave (Window* self) {
	puts(__FUNCTION__);
}

bool Window_keyDown (Window* self, unsigned keycode) {
	puts(__FUNCTION__);
	return false;
}

bool Window_keyUp (Window* self, unsigned keycode) {
	puts(__FUNCTION__);
	return false;
}

static View *Window_viewAtLocation (Window *self, int x, int y, int *returnX, int *returnY)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClass(self,Window);

	Point point = Point_new(x,y);
	unsigned nViews = $(self->viewsArray, count);
	for (unsigned i=0; i < nViews; i++) {
		View *view = $(self->viewsArray, at, i);
		if (!$(view, hidden)) {
			if (Rect_containsPoint (view->rect, point)) {
				if (returnX && returnY) {
					*returnX = x - view->rect.origin.x;
					*returnY = y - view->rect.origin.y;
				}
				return view;
			}
		}
	}

	return NULL;
}

long Window_message (Window* self, long message, Any *sender, long first, long second)
{
	if (!self) {
		return -1;
	}
	verifyCorrectClass(self,Window);

	int relativeX = 0, relativeY = 0;
	View *view = NULL;
	if (message == kPointerDownMessage 
	 || message == kPointerUpMessage 
	 || message == kPointerMovedMessage) {
		view = Window_viewAtLocation (self, first, second, &relativeX, &relativeY);
	}

	switch (message) {
	case kWindowPointerEntryMessage: {
		bool entered = first != 0;
		if (entered != self->pointerInside) {
			self->pointerInside = entered;
			Log_debug_printf (__FUNCTION__, "Pointer has %s", entered ? "entered" : "exited");
		}
	 } break;
	case kWindowVisibilityMessage: {
		Log_debug_printf (__FUNCTION__, "Window is %s", first ? "visible" : "hidden");
	 } break;
	case kKeyDownMessage: 
		Window_keyDown (self, first);
		break;
	case kKeyUpMessage: 
		Window_keyUp (self, first);
		break;
	case kQuittingNowMessage:
		Log_debug_printf (__FUNCTION__, "Window told program is exiting.");
		break;
	case kPointerDownMessage: {
		self->pointerX = first;
		self->pointerY = second;
		if (view) {
			$(view, message, kPointerDownMessage, self, relativeX, relativeY);
		}
		} break;
	case kPointerUpMessage: {
		if (view) {
			$(view, message, kPointerUpMessage, self, relativeX, relativeY);
		}
		int dx = first - self->pointerX;
		int dy = second - self->pointerY;
		float distance = sqrtf (dx * dx + dy * dy);
		if (distance < 5) {
			$(view, message, kClickedMessage, self, 0, 0);
		}
		} break;
	case kPointerMovedMessage: {
		if (view) {
			$(view, message, kPointerMovedMessage, self, relativeX, relativeY);
		}
		} break;
	}

	return 0;
}

void Window_addModel3D (Window* self, Model3D *model)
{
	if (!self || !model) {
		return;
	}
	verifyCorrectClass(self,Window);
	verifyCorrectClassOrSubclass(model,Model3D);
	$(self->modelsArray, append, model);
}

void Window_addView (Window* self, Any* view)
{
	if (!self || !view) {
		return;
	}
	verifyCorrectClass(self,Window);
	verifyCorrectClassOrSubclass(view,View);
	$(self->viewsArray, append, view);
}

void Window_setLightEnabled (Window *self, unsigned which, bool enabled)
{
	if (!self || which >= MAX_LIGHTS) {
		return;
	}
	verifyCorrectClass(self,Window);

	self->useLight[which] = enabled;
	self->needsRedraw = true;
}

void Window_setLightColors (Window *self, unsigned which, RGBA ambient, RGBA diffuse, RGBA specular)
{
	if (!self || which >= MAX_LIGHTS) {
		return;
	}
	verifyCorrectClass(self,Window);

	self->lightAmbientColors[which] = ambient;
	self->lightDiffuseColors[which] = diffuse;
	self->lightSpecularColors[which] = specular;
	self->needsRedraw = true;
}

void Window_setLightPosition (Window *self, unsigned which, Vector position)
{
	if (!self || which >= MAX_LIGHTS) {
		return;
	}
	verifyCorrectClass(self,Window);

	self->lightPositions[which] = position;
	self->needsRedraw = true;
}

void Window_setTopLeftMessage (Window *self, const char* message)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Window);

	self->topLeftMessage = message; // Can be NULL.
	self->needsRedraw = true;
}

void Window_setController (Window* self, Any* controller_)
{
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Window);

	if (self->controller) {
		Controller *controllerToRemove = self->controller;
		$(controllerToRemove, dismantle);
		$(controllerToRemove, setWindow, NULL);
		release(controllerToRemove);
	}

	Controller *controller = controller_;
	if (controller) {
		verifyCorrectClassOrSubclass(controller_,Controller);
		self->controller = retain(controller);
		$(controller, setWindow, self);
		$(controller, construct);
	} 
}

WindowClass* WindowClass_init (WindowClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(Window,describe);
	SET_OVERRIDDEN_METHOD_POINTER(Window,destroy);
	SET_OVERRIDDEN_METHOD_POINTER(Window,print);
	SET_OVERRIDDEN_METHOD_POINTER(Window,message);

	SET_INHERITED_METHOD_POINTER(Window,Object,equals);

	SET_METHOD_POINTER(Window,identifier);
	SET_METHOD_POINTER(Window,setIdentifier);
	SET_METHOD_POINTER(Window,resize);
	SET_METHOD_POINTER(Window,move);
	SET_METHOD_POINTER(Window,width);
	SET_METHOD_POINTER(Window,height);
	SET_METHOD_POINTER(Window,redraw);
	SET_METHOD_POINTER(Window,addModel3D);
	SET_METHOD_POINTER(Window,addView);
	SET_METHOD_POINTER(Window,moveCamera);
	SET_METHOD_POINTER(Window,setLightEnabled);
	SET_METHOD_POINTER(Window,setLightColors);
	SET_METHOD_POINTER(Window,setLightPosition);
	SET_METHOD_POINTER(Window,setController);
	SET_METHOD_POINTER(Window,setTopLeftMessage);

        VALIDATE_CLASS_STRUCT(class);
	return class;
}

Window *Window_newWith (Display *display, const char *title, unsigned width, unsigned height, int x, int y)
{
	ENSURE_CLASS_READY(Window);

	if (!display || !width || !height) {
		return NULL;
	}

	Window *self = allocate(Window);
	return Window_initWith (self, display, title, width, height, x, y);
}

Window *Window_initWith (Window *self,
			Display *display,
			const char *title,
			unsigned width,
			unsigned height,
			int x,
			int y)
{
	if (!self) { 
		return NULL;
	}
	if (!display || !width || !height) {
		release (self);
		return NULL;
	}
	verifyCorrectClassOrSubclass(display,Display);
	if (!$(display, canAddWindows)) {
		release (self);
		return NULL;
	}

	Window_init (self);

	self->x = x;
	self->y = y;
	self->width = width;
	self->height = height;
	self->title = strdup(title ?: ""); // XX use String
	self->backgroundColor = RGB_STEELBLUE;

	// RULE: Don't retain because Display retains Window.
	self->display = display; 

	char temp [256];
	snprintf (temp, sizeof(temp)-1, "Window (title \"%s\") is %ux%u@(%d,%d)", self->title, width, height, x, y);
	Log_debug (__FUNCTION__, temp);

#ifdef HAVE_OPENGL
	glutInitWindowSize (width, height);
	glutInitWindowPosition (x, y);
	int glWindow = glutCreateWindow (self->title);

        int actualWidth = glutGet (GLUT_WINDOW_WIDTH);
        int actualHeight = glutGet (GLUT_WINDOW_HEIGHT);
        
	if (actualWidth > 0 && actualHeight > 0 && (actualWidth != width || actualHeight != height)) {
		// NOTE: In X Windows, GLUT has a bug that prevents a window 
		// which the window manager forced to be smaller to display correctly 
		// until after the idle call.
		x = glutGet (GLUT_WINDOW_X);
		y = glutGet (GLUT_WINDOW_Y);
		width = actualWidth;
		height = actualHeight;

		Log_debug_printf (__FUNCTION__, "Window manager resized window to %ux%u", width, height);

        	//wasResizedAtOutset = true;
	}
	self->identifier = glWindow;
#endif

	$(display, addWindow, self);

	Window_resize (self, width, height);

	return self;
}

