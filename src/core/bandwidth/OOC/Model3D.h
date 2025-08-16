/*============================================================================
  Model3D, an object-oriented C mutable 3D model class.
  Copyright (C) 2023 by Zack T Smith.

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

#ifndef _OOC_MODEL3D_H
#define _OOC_MODEL3D_H

#include "Object.h"
#include "GraphicsTypes.h"

typedef struct { 
	float x; 
	float y;
	float z;
} Vector;

extern Vector Vector_crossProduct (Vector*, Vector*);
extern float Vector_dotProduct (Vector*, Vector*);
extern float Vector_magnitude (Vector*);
extern void Vector_normalize (Vector*);
extern void Vector_add (Vector*, Vector*);
extern Vector Vector_new (float x, float y, float z);
extern const char *Vector_toString (Vector);

typedef struct triangle3d {
	Vector a;
	Vector b;
	Vector c;
	Vector normal_a;
	Vector normal_b;
	Vector normal_c;
} Triangle;

extern Vector Triangle_normalVector (Triangle*);

struct model3d;

#define DECLARE_MODEL3D_INSTANCE_VARS(TYPE_POINTER) \
	Triangle *triangles; \
	size_t nTriangles; \
	size_t allocatedSize; \
	RGBA color;

#define DECLARE_MODEL3D_METHODS(TYPE_POINTER) \
	void (*addTriangle) (TYPE_POINTER, Triangle*); \
	void (*addModel) (TYPE_POINTER, TYPE_POINTER); \
	void (*translate) (TYPE_POINTER, float x, float y, float z); \
	void (*scale) (TYPE_POINTER, float s); \
	void (*clear) (TYPE_POINTER); \
	bool (*writeSTL) (TYPE_POINTER, const char*); \
	bool (*readSTL) (TYPE_POINTER, const char*); \
	void (*addCube) (TYPE_POINTER, float sideLength);\
	void (*addSphere) (TYPE_POINTER, float diameter, unsigned steps);\
	void (*addCylinder) (TYPE_POINTER, float height, float diameter, unsigned steps);\
	void (*addCone) (TYPE_POINTER, float height, float diameter, unsigned steps);

typedef struct model3dclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct model3d*)
        DECLARE_MODEL3D_METHODS(struct model3d*)
} Model3DClass;

extern Model3DClass *_Model3DClass;
extern Model3DClass* Model3DClass_init (Model3DClass*);

typedef struct model3d {
        Model3DClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct model3d*)
	DECLARE_MODEL3D_INSTANCE_VARS(struct model3d*)
} Model3D;

extern Model3D *Model3D_fromFile (const char*);
extern Model3D *Model3D_init (Model3D *self);
extern void Model3D_destroy (Any *);

#define degrees_to_radians(DEG) (M_PI * ((float)DEG) / 180.f)

#endif

