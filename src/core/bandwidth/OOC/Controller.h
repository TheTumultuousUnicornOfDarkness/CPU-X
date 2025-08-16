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

#ifndef _OOC_CONTROLLER_H
#define _OOC_CONTROLLER_H

#include "Window.h"

#define DECLARE_CONTROLLER_INSTANCE_VARS(FOO) \
	Window *window;

#define DECLARE_CONTROLLER_METHODS(TYPE_POINTER) \
	bool (*construct) (TYPE_POINTER); /* If returns false => construct failed */ \
	bool (*update) (TYPE_POINTER); /* If returns false => app should exit */ \
	void (*layout) (TYPE_POINTER); \
	void (*dismantle) (TYPE_POINTER); \
	void (*setWindow) (TYPE_POINTER, Any*); \
	Window *(*window) (TYPE_POINTER); 

struct controller;

typedef struct controllerclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct controller*)
        DECLARE_CONTROLLER_METHODS(struct controller*)
} ControllerClass;

extern ControllerClass *_ControllerClass;
extern ControllerClass* ControllerClass_init (ControllerClass*);

typedef struct controller {
        ControllerClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct controller*)
	DECLARE_CONTROLLER_INSTANCE_VARS(struct controller*)
} Controller;

extern void Controller_destroy (Any *);
extern Controller *Controller_init (Controller *self);

#endif
