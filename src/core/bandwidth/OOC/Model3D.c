/*============================================================================
  Model3D, an object-oriented C mutable 3D vector class.
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

#include "Object.h"
#include "Log.h"
#include "Model3D.h"
#include "Utility.h"

#include <math.h>

Model3DClass *_Model3DClass = NULL;

#define INITIAL_NUMBER_OF_TRIANGLES 1024

Model3D *Model3D_init (Model3D *self)
{
	ENSURE_CLASS_READY(Model3D);
	if (!self) {
		return NULL;
	}
	Object_init ((Object*) self);
	self->is_a = _Model3DClass;
	self->triangles = malloc(sizeof(Triangle) * INITIAL_NUMBER_OF_TRIANGLES);
	self->nTriangles = 0;
	self->allocatedSize = INITIAL_NUMBER_OF_TRIANGLES;

	return self;
}

Vector Vector_new (float x, float y, float z) {
	Vector v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

static char vector_string_buffer[64];
const char *Vector_toString (Vector v) {
	snprintf (vector_string_buffer, sizeof(vector_string_buffer), "%g, %g, %g", 
		v.x, v.y, v.z);
	return vector_string_buffer;
}

// Note, Vector is a struct, not an object.
void Vector_normalize (Vector *self)
{
	if (!self) {
		return;
	}
	float x2 = self->x * self->x;
	float y2 = self->y * self->y;
	float z2 = self->z * self->z;
	float magnitude = sqrt (x2 + y2 + z2);
	self->x /= magnitude;
	self->y /= magnitude;
	self->z /= magnitude;
}

float Vector_magnitude (Vector *self)
{
	if (!self) {
		return 0.f;
	}
	float x2 = self->x * self->x;
	float y2 = self->y * self->y;
	float z2 = self->z * self->z;
	return sqrt (x2 + y2 + z2);
}

bool Vector_equals (Vector *self, Vector *other)
{
	if (!self || !other) {
		return false;
	}
	return self->x == other->x && self->y == other->y && self->z == other->z;
}

Vector Vector_crossProduct (Vector *self, Vector *other)
{
	if (!self || !other) {
		Vector result;
		result.x = 0;
		result.y = 0;
		result.z = 0;
		return result;
	}
	float cx = self->y * other->z - self->z * other->y;
	float cy = self->z * other->x - self->x * other->z;
	float cz = self->x * other->y - self->y * other->x;
	Vector result;
	result.x = cx;
	result.y = cy;
	result.z = cz;
	return result;
}

float Vector_dotProduct (Vector *self, Vector *other)
{
	if (!self || !other) {
		return 0.f;
	}

	return self->x * other->x + self->y * other->y;
}

void Vector_add (Vector *self, Vector *other)
{
	if (!self || !other) {
		return;
	}
	self->x += other->x;
	self->y += other->y;
	self->z += other->z;
}

void Model3D_clear (Model3D *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	if (self->allocatedSize > INITIAL_NUMBER_OF_TRIANGLES) {
		free (self->triangles);
		self->triangles = NULL;
		Model3D_init (self);
	} else {
		self->nTriangles = 0;
	}
}

void Model3D_print (Model3D* self, FILE* output)
{
        if (!self) {
                return;
        }
        verifyCorrectClass(self,Model3D);

        if (!output) {
		output = stdin;
	}

	fprintf (output, "%s(%lu triangles)", $(self, className), (unsigned long)self->nTriangles);
}

void Model3D_addTriangle (Model3D *self, Triangle* triangle)
{
	if (!self || !triangle) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	if (self->nTriangles >= self->allocatedSize) {
		size_t newCount = self->allocatedSize + INITIAL_NUMBER_OF_TRIANGLES;
		Triangle* newTriangles = realloc (self->triangles, newCount * sizeof(Triangle));
		if (!newTriangles) {
			Log_perror (__FUNCTION__, "realloc");
			return;
		}
		self->triangles = newTriangles;
		self->allocatedSize = newCount;
	}

	self->triangles[self->nTriangles++] = *triangle;
}

void Model3D_addModel (Model3D *self, Model3D* model)
{
	if (!self || !model) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);
	verifyCorrectClassOrSubclass(model,Model3D);

	for (unsigned i=0; i < model->nTriangles; i++) {
		Triangle t = self->triangles[i];
		Model3D_addTriangle (self, &t);
	}
}

bool Model3D_writeSTL (Model3D *self, const char* path)
{
	if (!self || !path) {
		return false;
	}

	verifyCorrectClassOrSubclass(self,Model3D);

	FILE *f = fopen (path, "wb");
	if (!f) {
		perror ("fopen");
		return false;
	}

	// Write ASCII model file.

	const char *name = "something";
	fprintf (f, "solid %s\n", name);

	for (unsigned i=0; i < self->nTriangles; i++) {
		Triangle t = self->triangles[i];

		// STL uses one normal per triangle.
		Vector n = Triangle_normalVector(&t);

		fprintf (f, "facet normal %.5f %.5f %.5f\n", n.x, n.y, n.z);
		fprintf (f, " outer loop\n");
		fprintf (f, "  vertex %.5f %.5f %.5f\n", t.a.x, t.a.y, t.a.z);
		fprintf (f, "  vertex %.5f %.5f %.5f\n", t.b.x, t.b.y, t.b.z);
		fprintf (f, "  vertex %.5f %.5f %.5f\n", t.c.x, t.c.y, t.c.z);
		fprintf (f, " endloop\n");
		fprintf (f, "endfacet\n");
	}
	
	fprintf (f, "endsolid %s\n", name);
	fflush (f);
	fclose (f);
	return true;
}

bool Model3D_readSTL(Model3D *self, const char* path) 
{
	return false;
}

void Model3D_destroy (Any *self)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	Model3D *model = self;
	if (model->triangles) {
		free(model->triangles);
		model->triangles = NULL;
	}

	Object_destroy (self);
}

void Model3D_translate (Model3D *self, float x, float y, float z)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	for (unsigned i=0; i < self->nTriangles; i++) {
		Triangle triangle = self->triangles[i];
		triangle.a.x += x;
		triangle.b.x += x;
		triangle.c.x += x;
		triangle.a.y += y;
		triangle.b.y += y;
		triangle.c.y += y;
		triangle.a.z += z;
		triangle.b.z += z;
		triangle.c.z += z;
		self->triangles[i] = triangle;
	}
}

void Model3D_scale (Model3D *self, float s)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	for (unsigned i=0; i < self->nTriangles; i++) {
		Triangle triangle = self->triangles[i];
		triangle.a.x *= s;
		triangle.b.x *= s;
		triangle.b.x *= s;
		triangle.a.y *= s;
		triangle.b.y *= s;
		triangle.b.y *= s;
		triangle.a.z *= s;
		triangle.b.z *= s;
		triangle.b.z *= s;
		self->triangles[i] = triangle;
	}
}

Vector Triangle_normalVector (Triangle* t)
{
	// Inspired by https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
	// Deltas
        float ux = t->b.x - t->a.x;
	float uy = t->b.y - t->a.y;
	float uz = t->b.z - t->a.z;
	float vx = t->c.x - t->a.x;
	float vy = t->c.y - t->a.y;
	float vz = t->c.z - t->a.z;

	// Normal is cross product
	float nx = uy * vz - uz * vy;
	float ny = uz * vx - ux * vz;
	float nz = ux * vy - uy * vx;

	// Normalize the normal vector
        float magnitude = sqrtf (nx*nx + ny*ny + nz*nz);
	Vector normal;
	if (magnitude > 0.f) {
		normal.x = nx / magnitude;
		normal.y = ny / magnitude;
		normal.z = nz / magnitude;
	} else {
		normal.x = 0.f;
		normal.y = 0.f;
		normal.z = 0.f;
	}
	return normal;
}

static void Model_addXPlaneRect (Model3D *self, float x, float ymin, float ymax, float zmin, float zmax,
				bool use_positive_normals)
{
	Triangle t;
	t.a.x = x;
	t.a.y = ymin;
	t.a.z = zmin;
	t.b.x = x;
	t.b.y = ymin;
	t.b.z = zmax;
	t.c.x = x;
	t.c.y = ymax;
	t.c.z = zmin;
	float n = use_positive_normals ? 1 : -1;
	t.normal_a.x = n;
	t.normal_a.y = 0;
	t.normal_a.z = 0;
	t.normal_b.x = n;
	t.normal_b.y = 0;
	t.normal_b.z = 0;
	t.normal_c.x = n;
	t.normal_c.y = 0;
	t.normal_c.z = 0;
	Model3D_addTriangle (self, &t);
	t.a.x = x;
	t.a.y = ymax;
	t.a.z = zmin;
	t.b.x = x;
	t.b.y = ymin;
	t.b.z = zmax;
	t.c.x = x;
	t.c.y = ymax;
	t.c.z = zmax;
	Model3D_addTriangle (self, &t);
}

static void Model_addYPlaneRect (Model3D *self, float y, float xmin, float xmax, float zmin, float zmax,
				bool use_positive_normals)
{
	Triangle t;
	t.a.x = xmin;
	t.a.y = y;
	t.a.z = zmin;
	t.b.x = xmin;
	t.b.y = y;
	t.b.z = zmax;
	t.c.x = xmax;
	t.c.y = y;
	t.c.z = zmin;
	float n = use_positive_normals ? 1 : -1;
	t.normal_a.x = 0;
	t.normal_a.y = n;
	t.normal_a.z = 0;
	t.normal_b.x = 0;
	t.normal_b.y = n;
	t.normal_b.z = 0;
	t.normal_c.x = 0;
	t.normal_c.y = n;
	t.normal_c.z = 0;
	Model3D_addTriangle (self, &t);
	t.a.x = xmax;
	t.a.y = y;
	t.a.z = zmin;
	t.b.x = xmin;
	t.b.y = y;
	t.b.z = zmax;
	t.c.x = xmax;
	t.c.y = y;
	t.c.z = zmax;
	Model3D_addTriangle (self, &t);
}

static void Model_addZPlaneRect (Model3D *self, float z, float xmin, float xmax, float ymin, float ymax,
				bool use_positive_normals)
{
	Triangle t;
	t.a.x = xmin;
	t.a.y = ymin;
	t.a.z = z;
	t.b.x = xmin;
	t.b.y = ymax;
	t.b.z = z;
	t.c.x = xmax;
	t.c.y = ymin;
	t.c.z = z;
	float n = use_positive_normals ? 1 : -1;
	t.normal_a.x = 0;
	t.normal_a.y = 0;
	t.normal_a.z = n;
	t.normal_b.x = 0;
	t.normal_b.y = 0;
	t.normal_b.z = n;
	t.normal_c.x = 0;
	t.normal_c.y = 0;
	t.normal_c.z = n;
	Model3D_addTriangle (self, &t);
	t.a.x = xmax;
	t.a.y = ymin;
	t.a.z = z;
	t.b.x = xmin;
	t.b.y = ymax;
	t.b.z = z;
	t.c.x = xmax;
	t.c.y = ymax;
	t.c.z = z;
	Model3D_addTriangle (self, &t);
}

void Model3D_addCube (Model3D *self, float size)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	float half = size / 2.f;

	// Left
	Model_addXPlaneRect (self, -half, -half, half, -half, half, false);
	// Right
	Model_addXPlaneRect (self, half, half, -half, -half, half, true);

	// Lower
	Model_addYPlaneRect (self, -half, -half, half, -half, half, false);
	// Upper
	Model_addYPlaneRect (self, half, half, -half, -half, half, true);

	// Far
	Model_addZPlaneRect (self, -half, -half, half, -half, half, false);
	// Near
	Model_addZPlaneRect (self, half, half, -half, -half, half, true);
}

void Model3D_addYDisc (Model3D *self, float y0, float diameter, bool use_positive_normals)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	float angle_delta = 12.f;
	float desired_radius = diameter / 2.f;
	float ny = use_positive_normals ? 1.f : -1.f;

	for (float angle = 0.f; angle < 360.0f; angle += angle_delta) {
		float horizontal_radians0 = degrees_to_radians(angle);
		float horizontal_radians1 = degrees_to_radians(angle+angle_delta);
		float x0 = sin (horizontal_radians0);
		float z0 = cos (horizontal_radians0);
		float x1 = sin (horizontal_radians1);
		float z1 = cos (horizontal_radians1);

		x0 *= desired_radius;
		z0 *= desired_radius;
		x1 *= desired_radius;
		z1 *= desired_radius;

		float x3 = 0.f;
		float y3 = y0;
		float z3 = 0.f;
		
		Triangle t;
		t.a.x = x1;
		t.a.y = y0;
		t.a.z = z1;
		t.b.x = x0;
		t.b.y = y0;
		t.b.z = z0;
		t.c.x = x3;
		t.c.y = y3;
		t.c.z = z3;

		t.normal_a.x = 0;
		t.normal_a.y = ny;
		t.normal_a.z = 0;
		t.normal_b.x = 0;
		t.normal_b.y = ny;
		t.normal_b.z = 0;
		t.normal_c.x = 0;
		t.normal_c.y = ny;
		t.normal_c.z = 0;

		Model3D_addTriangle (self, &t);
	}
}

void Model3D_addCone (Model3D *self, float height, float diameter, unsigned steps)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	if (!steps) {
		steps = 360;
	}

	float angle_delta = 360.f / (float)steps;
	float desired_radius = diameter / 2.f;

	// Tip of the cone
	float x2 = 0.f;
	float y2 = height/2.f;
	float z2 = 0.f;

	float y0 = -height/2.f;

	// Create the cone, going clockwise
	for (float angle = 0.f; angle < 360.0f; angle += angle_delta) {
		float horizontal_radians0 = degrees_to_radians(angle);
		float horizontal_radians1 = degrees_to_radians(angle+angle_delta);
		float x0 = sin (horizontal_radians0);
		float z0 = cos (horizontal_radians0);
		float x1 = sin (horizontal_radians1);
		float z1 = cos (horizontal_radians1);

		Triangle t1;

		x0 *= desired_radius;
		z0 *= desired_radius;
		x1 *= desired_radius;
		z1 *= desired_radius;

		t1.a.x = x0;
		t1.a.y = y0;
		t1.a.z = z0;
		t1.b.x = x1;
		t1.b.y = y0;
		t1.b.z = z1;
		t1.c.x = x2;
		t1.c.y = y2;
		t1.c.z = z2;

		/*        top         
		 *       x2,y2,z2
		 *         |\	
		 *         | \	
		 *         |  \	
		 *         |   \	
		 *         |    \	
		 *         | t1  \	
		 *         |      \	
		 *         |       \	
		 *         |        \	
		 *         |         \	
		 *       x0,z0---y0--x1,z1
		 *      bottom       bottom
		 */

		Vector normal = Triangle_normalVector (&t1);
		t1.normal_a.x = normal.x;
		t1.normal_a.y = normal.y;
		t1.normal_a.z = normal.z;
		t1.normal_b.x = normal.x;
		t1.normal_b.y = normal.y;
		t1.normal_b.z = normal.z;
		t1.normal_c.x = normal.x;
		t1.normal_c.y = normal.y;
		t1.normal_c.z = normal.z;

		Model3D_addTriangle (self, &t1);
	}

	Model3D_addYDisc (self, y0, diameter, false);
}

void Model3D_addSphere (Model3D *self, float diameter, unsigned steps)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	if (!steps) {
		steps = 360;
	}

	float angle_delta = 360.f / (float)steps;
	float desired_radius = diameter / 2.f;

	// Proceed from bottom to top.
	// Vertically, the y-axis radius follows a semi-circle.
	for (float yangle = -90.f; yangle < 90.f; yangle += angle_delta) {
		float vertical_radians0 = degrees_to_radians(yangle);
		float vertical_radians1 = degrees_to_radians(yangle+angle_delta);
		float innerx0 = sin (vertical_radians0);
		float innery0 = cos (vertical_radians0);
		float innerx1 = sin (vertical_radians1);
		float innery1 = cos (vertical_radians1);
		float radius0 = sqrt(innerx0 * innerx0 + innery0 * innery0);
		float radius1 = sqrt(innerx1 * innerx1 + innery1 * innery1);
		float y0 = innerx0;
		float y1 = innerx1;
		radius0 = innery0;
		radius1 = innery1;

		for (float angle = 0.f; angle < 360.f; angle += angle_delta) {
			// Horizontally, on the y plane, the radius follows a full circle.
			float horizontal_radians0 = degrees_to_radians(angle);
			float horizontal_radians1 = degrees_to_radians(angle+angle_delta);

			float x0 = sin (horizontal_radians0);
			float z0 = cos (horizontal_radians0);
			float x1 = sin (horizontal_radians1);
			float z1 = cos (horizontal_radians1);

			float top_x0 = x0 * radius1;
			float top_z0 = z0 * radius1;
			float top_x1 = x1 * radius1;
			float top_z1 = z1 * radius1;
			float bottom_x0 = x0 * radius0;
			float bottom_z0 = z0 * radius0;
			float bottom_x1 = x1 * radius0;
			float bottom_z1 = z1 * radius0;

			if (top_x1 != top_x0 && top_z1 != top_z0) {
				Triangle t1;
				// Going clockwise 
				t1.a.x = desired_radius * top_x0;
				t1.a.y = desired_radius * y1;
				t1.a.z = desired_radius * top_z0;
				t1.b.x = desired_radius * top_x1;
				t1.b.y = desired_radius * y1;
				t1.b.z = desired_radius * top_z1;
				t1.c.x = desired_radius * bottom_x1;
				t1.c.y = desired_radius * y0;
				t1.c.z = desired_radius * bottom_z1;
				t1.normal_a.x = top_x0;
				t1.normal_a.y = y1;
				t1.normal_a.z = top_z0;
				t1.normal_b.x = top_x1;
				t1.normal_b.y = y1;
				t1.normal_b.z = top_z1;
				t1.normal_c.x = bottom_x1;
				t1.normal_c.y = y0;
				t1.normal_c.z = bottom_z1;
				Model3D_addTriangle (self, &t1);
			}

//        top         top
//       x0,z0---y1--x1,z1
//         |\         |    
//         | \        |    
//         |  \       |    
//         |   \  t1  |    
//         |    \     |    
//         | t2  \    |    
//         |      \   |    
//         |       \  |    
//         |        \ |    
//         |         \|    
//       x0,z0---y0--x1,z1
//      bottom       bottom
//
			if (bottom_x1 != bottom_x0 && bottom_z1 != bottom_z0) {
				Triangle t2;
				// Going clockwise
				t2.a.x = desired_radius * bottom_x1;
				t2.a.y = desired_radius * y0;
				t2.a.z = desired_radius * bottom_z1;
				t2.b.x = desired_radius * bottom_x0;
				t2.b.y = desired_radius * y0;
				t2.b.z = desired_radius * bottom_z0;
				t2.c.x = desired_radius * top_x0;
				t2.c.y = desired_radius * y1;
				t2.c.z = desired_radius * top_z0;
				t2.normal_a.x = bottom_x1;
				t2.normal_a.y = y0;
				t2.normal_a.z = bottom_z1;
				t2.normal_b.x = bottom_x0;
				t2.normal_b.y = y0;
				t2.normal_b.z = bottom_z0;
				t2.normal_c.x = top_x0;
				t2.normal_c.y = y1;
				t2.normal_c.z = top_z0;
				Model3D_addTriangle (self, &t2);
			}
		}
	}
}

void Model3D_addCylinder (Model3D *self, float height, float diameter, unsigned steps)
{
	if (!self) {
		return;
	}
	verifyCorrectClassOrSubclass(self,Model3D);

	if (!steps) {
		steps = 360;
	}

	float angle_delta = 360.f / (float)steps;
	float desired_radius = diameter / 2.f;

	float y1 = height/2.f;
	float y0 = -height/2.f;

	for (float angle = 0.f; angle < 360.0f; angle += angle_delta) {
		float horizontal_radians0 = degrees_to_radians(angle);
		float horizontal_radians1 = degrees_to_radians(angle + angle_delta);
		float x0 = sin (horizontal_radians0);
		float z0 = cos (horizontal_radians0);
		float x1 = sin (horizontal_radians1);
		float z1 = cos (horizontal_radians1);

		Triangle t1;
		Triangle t2;
		t1.normal_a.x = x0;
		t1.normal_a.y = 0;
		t1.normal_a.z = z0;
		t1.normal_b.x = x1;
		t1.normal_b.y = 0;
		t1.normal_b.z = z1;
		t1.normal_c.x = x1;
		t1.normal_c.y = 0;
		t1.normal_c.z = z1;

		t2.normal_a.x = x1;
		t2.normal_a.y = 0;
		t2.normal_a.z = z1;
		t2.normal_b.x = x0;
		t2.normal_b.y = 0;
		t2.normal_b.z = z0;
		t2.normal_c.x = x0;
		t2.normal_c.y = 0;
		t2.normal_c.z = z0;

		x0 *= desired_radius;
		z0 *= desired_radius;
		x1 *= desired_radius;
		z1 *= desired_radius;

		// Going clockwise
		t1.a.x = x0;
		t1.a.y = y1;
		t1.a.z = z0;
		t1.b.x = x1;
		t1.b.y = y1;
		t1.b.z = z1;
		t1.c.x = x1;
		t1.c.y = y0;
		t1.c.z = z1;
		Model3D_addTriangle (self, &t1);

		//        top         top
		//       x0,z0---y1--x1,z1
		//         |\         |    
		//         | \        |    
		//         |  \       |    
		//         |   \  t1  |    
		//         |    \     |    
		//         | t2  \    |    
		//         |      \   |    
		//         |       \  |    
		//         |        \ |    
		//         |         \|    
		//       x0,z0---y0--x1,z1
		//      bottom       bottom
		
		// Going clockwise
		t2.a.x = x1;
		t2.a.y = y0;
		t2.a.z = z1;
		t2.b.x = x0;
		t2.b.y = y0;
		t2.b.z = z0;
		t2.c.x = x0;
		t2.c.y = y1;
		t2.c.z = z0;
		Model3D_addTriangle (self, &t2);
	}

	Model3D_addYDisc (self, y1, diameter, true);
	Model3D_addYDisc (self, y0, diameter, false);
}

Model3DClass *Model3DClass_init (Model3DClass* class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(Model3D,print);
	SET_OVERRIDDEN_METHOD_POINTER(Model3D,destroy);

	SET_METHOD_POINTER(Model3D,addTriangle);
	SET_METHOD_POINTER(Model3D,addModel);
	SET_METHOD_POINTER(Model3D,clear);
	SET_METHOD_POINTER(Model3D,writeSTL);
	SET_METHOD_POINTER(Model3D,readSTL);
	SET_METHOD_POINTER(Model3D,addCube);
	SET_METHOD_POINTER(Model3D,addSphere);
	SET_METHOD_POINTER(Model3D,addCone);
	SET_METHOD_POINTER(Model3D,addCylinder);
	SET_METHOD_POINTER(Model3D,translate);
	SET_METHOD_POINTER(Model3D,scale);
	
        VALIDATE_CLASS_STRUCT(class);
	return class;
}

Model3D *Model3D_fromFile (const char* path)
{
	if (!path) {
		return NULL;
	}
	if (!file_exists (path)) {
		Log_warning(__FUNCTION__, "No such file.");
		return NULL;
	}
	if (!has_suffix(path, ".stl")) {
		Log_warning(__FUNCTION__, "Only STL can be read.");
		return NULL;
	}

	Model3D *model = new(Model3D);
	if (!Model3D_readSTL (model, path)) {
		release(model);
		return NULL;
	}

	return model;
}

