/*============================================================================
  DateTime, an object-oriented C date/time class.
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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "DateTime.h"

static const int days_per_month[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
};

DateTimeClass *_DateTimeClass = NULL;

void DateTime_destroy (Any *self_)
{
	DEBUG_DESTROY;
	if (!self_)
		return;
	verifyCorrectClassOrSubclass(self_, DateTime);
}

DateTime *DateTime_previousDay (DateTime *self)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClass(self,DateTime);

	int d = self->day;
	int m = self->month;
	int y = self->year;
	
	d--;
	if (d < 1) {
		m--;
		if (m < 1) {
			m = 12;
			y--;
		}
		
		int n_days = days_per_month [m-1];
		
		if (!(y & 3) && m == 2)
			n_days = 29;
		
		d = n_days;
	}
	
	DateTime *returnDate = DateTime_new();
	returnDate->day = d;
	returnDate->month = m;
	returnDate->year = y;
	return returnDate;
}

DateTime *DateTime_nextDay (DateTime *self)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClass(self,DateTime);

	int d = self->day;
	int m = self->month;
	int y = self->year;
	
	int n_days = days_per_month [m-1];
	
	if (!(y & 3) && m == 2)
		n_days = 29;
	
	d++;
	if (d > n_days) {
		d = 1;
		m++;
		if (m > 12) {
			m = 1;
			y++;
		}
	}
	
	DateTime *returnDate = DateTime_new();
	returnDate->day = d;
	returnDate->month = m;
	returnDate->year = y;
	return returnDate;
}

const char *DateTime_dateString (DateTime *self, char separator)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClass(self,DateTime);

	static char string [64];
	if (separator) {
		snprintf (string, sizeof(string), "%04d%c%02d%c%02d", self->year, separator, self->month, separator, self->day);
	}
	else {
		snprintf (string, sizeof(string), "%04d%02d%02d", self->year, self->month, self->day);
	}
	return (const char*) string;
}

const char *DateTime_timeString (DateTime *self, char separator)
{
	if (!self) {
		return NULL;
	}
	verifyCorrectClass(self,DateTime);

	static char string [64];
	if (separator) {
		snprintf (string, sizeof(string), "%04d%c%02d%c%02d", self->hour, separator, self->minute, separator, self->seconds);
	}
	else {
		snprintf (string, sizeof(string), "%04d%02d%02d", self->hour, self->minute, self->seconds);
	}
	return (const char*) string;
}

// Class method
DateTime *DateTime_now ()
{
        time_t t;
        t = time (NULL);

        struct tm *t2;
        t2 = localtime (&t);

	DateTime *self = DateTime_new();
        self->dayOfWeek = t2->tm_wday;
        self->seconds = t2->tm_sec;
        self->minute = t2->tm_min;
        self->hour = t2->tm_hour;
        self->day = t2->tm_mday;
        self->month = t2->tm_mon + 1;
        self->year = 1900 + t2->tm_year;

        return self;
}

// Class method
uint64_t DateTime_getMicrosecondTime ()
{
	struct timeval tv;
	struct timezone tz;
	memset (&tv, 0, sizeof(struct timeval));
	memset (&tz, 0, sizeof(struct timezone));
	gettimeofday (&tv, &tz);
	uint64_t microseconds = (uint64_t)tv.tv_sec;
	microseconds *= 1000000LU;
	microseconds += (uint64_t)tv.tv_usec;
	return microseconds;
}

// Class method
uint64_t DateTime_getMillisecondTime ()
{
       struct timeval t;
       struct timezone tz;
       memset (&tz, 0, sizeof(struct timezone));
       gettimeofday (&t, &tz);
       return (uint64_t) (t.tv_sec * 1000 + t.tv_usec / 1000);
}

// Class method
int DateTime_getWeekDayNumber ()
{
        time_t t = time (NULL);

        struct tm *timestruct = localtime (&t);

        return timestruct->tm_wday;
}

// Class method
const char *DateTime_getTodayString (char separator)
{
	static char string [64];
	int y, m, d;
	DateTime_getDate (&y, &m, &d);
	if (separator)
		snprintf (string, sizeof(string)-1, "%04d%c%02d%c%02d", y, separator, m, separator, d);
	else
		snprintf (string, sizeof(string)-1, "%04d%02d%02d", y, m, d);
	return string;
}

// Class method
void DateTime_getDate (int *year, int *month, int *day)
{
        time_t t = time (NULL);

        struct tm *timestruct = localtime (&t);

	if (year)
		*year = 1900 + timestruct->tm_year;
        if (month)
		*month = timestruct->tm_mon + 1;
	if (day)
		*day = timestruct->tm_mday;
}

DateTime *DateTime_new () 
{
	return new(DateTime);
}

static bool DateTime_equals (DateTime *self, void *other_) 
{ 
	if (!self || !other_) 
		return false;
	verifyCorrectClass(self,DateTime);
	DateTime *other = (DateTime*)other_;
	verifyCorrectClass(other,DateTime);
	if (self->is_a != other->is_a)
		return false;

	return (self->year != other->year 
	     || self->month != other->month
	     || self->day != other->day
	     || self->hour != other->hour
	     || self->minute != other->minute
	     || self->seconds != other->seconds) ? false : true;
}

static void DateTime_print (DateTime* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,DateTime);

	if (!outputFile)
		outputFile = stdout;

	const char *dateString = DateTime_dateString (self, '/');
	const char *timeString = DateTime_timeString (self, ':');
	fprintf (outputFile, "%s %s", dateString, timeString);
}

static void DateTime_describe (DateTime* self, FILE *outputFile) 
{ 
	if (!self) {
		return;
	}
	verifyCorrectClass(self,DateTime);

	if (!outputFile)
		outputFile = stdout;

	fprintf (outputFile, "%s", $(self, className));
}

DateTime* DateTime_init (DateTime *self)
{
	Object_init ((Object*) self);
	self->is_a = _DateTimeClass;

	return self;
}

DateTimeClass* DateTimeClass_init (DateTimeClass *class)
{
	SET_SUPERCLASS(Object);

	SET_OVERRIDDEN_METHOD_POINTER(DateTime,destroy);
	SET_OVERRIDDEN_METHOD_POINTER(DateTime,describe);
	SET_OVERRIDDEN_METHOD_POINTER(DateTime,print);
	SET_OVERRIDDEN_METHOD_POINTER(DateTime,equals);

	SET_METHOD_POINTER(DateTime,previousDay);
	SET_METHOD_POINTER(DateTime,nextDay);
	SET_METHOD_POINTER(DateTime,dateString);
	SET_METHOD_POINTER(DateTime,timeString);
	
	VALIDATE_CLASS_STRUCT(class);
	return class;
}

