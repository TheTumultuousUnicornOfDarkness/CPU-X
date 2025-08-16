/*=========================================================================
  DateTime, an object-oriented C date and time class.
  This file is part of OOC.
 
  Copyright (C) 2019 by Zack T Smith.

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

#ifndef _DATETIME
#define _DATETIME

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "Object.h"

#define DECLARE_DATETIME_INSTANCE_VARS(FOO) \
        unsigned int year; \
        unsigned int month; \
        unsigned int day; \
        unsigned int hour; \
        unsigned int minute; \
        unsigned int seconds; \
        unsigned int dayOfWeek; 

struct datetime;

#define DECLARE_DATETIME_METHODS(TYPE_POINTER) \
	TYPE_POINTER (*previousDay) (TYPE_POINTER); \
	TYPE_POINTER (*nextDay )(TYPE_POINTER); \
	const char *(*dateString) (TYPE_POINTER, char); \
	const char *(*timeString) (TYPE_POINTER, char); 

typedef struct datetimeclass {
	DECLARE_OBJECT_CLASS_VARS
        DECLARE_OBJECT_METHODS(struct datetime*)
        DECLARE_DATETIME_METHODS(struct datetime*)
} DateTimeClass;

typedef struct datetime {
        DateTimeClass *is_a;
	DECLARE_OBJECT_INSTANCE_VARS(struct datetime*)
	DECLARE_DATETIME_INSTANCE_VARS(struct datetime*)
} DateTime;

DateTime *DateTime_now ();

extern DateTimeClass *_DateTimeClass;
extern DateTimeClass* DateTimeClass_init (DateTimeClass*);

extern DateTime *DateTime_new ();
extern void DateTime_destroy (Any *);
extern DateTime *DateTime_init (DateTime *self);
extern const char *DateTime_dateString (DateTime *, char);
extern DateTime *DateTime_previousDay (DateTime *);
extern DateTime *DateTime_nextDay (DateTime *);

// Class methods
extern uint64_t DateTime_getMicrosecondTime ();
extern uint64_t DateTime_getMillisecondTime ();
extern void DateTime_getDate (int *year, int *month, int *day);
extern const char *DateTime_getTodayString (char);
extern int DateTime_getWeekDayNumber ();

#endif

