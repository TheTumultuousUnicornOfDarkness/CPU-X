/*============================================================================
  Controller, an object-oriented C user interface controller class.
  Copyright (C) 2024 by Zack T Smith.

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

#include "Controller.h"

#include <stdlib.h>

ControllerClass *_ControllerClass = NULL;

void Controller_destroy (Any *self_)
{
	DEBUG_DESTROY;
	if (!self_) {
		return;
	}
	verifyCorrectClassOrSubclass(self_,Controller);
}

static void Controller_print (Controller* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Controller);

	if (!outputFile) {
		outputFile = stdout;
	}
}

static void Controller_describe (Controller* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,Controller);

	if (!outputFile) {
		outputFile = stdout;
	}

	fprintf (outputFile, "%s", $(self, className));
}

static bool Controller_construct (Controller* self)
{ 
	return false;
}

static void Controller_layout (Controller* self)
{
}

static bool Controller_update (Controller* self)
{ 
	return false;
}

static void Controller_dismantle (Controller* self)
{ 
}

void Controller_setWindow (Controller* self, Any* window)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Controller);
	verifyCorrectClassOrSubclass(window,Window);
	self->window = window; // Don't retain.
}

static Window *Controller_window (Controller* self)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClassOrSubclass(self,Controller);
	return self->window;
}

ControllerClass* ControllerClass_init (ControllerClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(Controller,describe);
	SET_OVERRIDDEN_METHOD_POINTER(Controller,print);

	SET_METHOD_POINTER(Controller,construct);
	SET_METHOD_POINTER(Controller,update);
	SET_METHOD_POINTER(Controller,layout);
	SET_METHOD_POINTER(Controller,dismantle);
	SET_METHOD_POINTER(Controller,setWindow);
	SET_METHOD_POINTER(Controller,window);
	
        VALIDATE_CLASS_STRUCT(class);
	return class;
}

Controller* Controller_init (Controller *self)
{
	ENSURE_CLASS_READY(Controller);

	if (self) {
		Object_init ((Object*) self);
		self->is_a = _ControllerClass;
	}

	return self;
}

