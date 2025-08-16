/*============================================================================
  RectStack, an object-oriented C mutable vector class.
  Copyright (C) 2022 by Zack T Smith.

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

#include <stdint.h>

#include "RectStack.h"

#define StackType RectStack
#define ValueType Rect

void RectStack_print (RectStack* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self, StackType);

	if (!outputFile) {
		outputFile = stdout;
	}

	fprintf (outputFile, "%s(%lu)\n", $(self, className), (unsigned long) self->count); 

	int index = self->count - 1;
	while (index >= 0) {
		Rect rect = self->array[index];
		printf ("\tItem %d = ", index);
		Rect_print (rect, NULL);
		putchar ('\n');
		index--;
	}
}

#include "StackTemplate.c"

