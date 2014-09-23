#
#   DMI Decode
#   BIOS Decode
#   VPD Decode
#
#   Copyright (C) 2000-2002 Alan Cox <alan@redhat.com>
#   Copyright (C) 2002-2007 Jean Delvare <khali@linux-fr.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#

CC      = gcc
CFLAGS  = -W -Wall -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual \
          -Wcast-align -Wwrite-strings -Wmissing-prototypes -Winline -Wundef
#CFLAGS += -DBIGENDIAN
#CFLAGS += -DALIGNMENT_WORKAROUND

# When debugging, disable -O2 and enable -g.
CFLAGS += -O2
#CFLAGS += -g

LIBFLAGS = -fPIC

# Pass linker flags here
LDFLAGS =

DESTDIR =
prefix  = /usr/local
sbindir = $(prefix)/sbin
mandir  = $(prefix)/share/man
man8dir = $(mandir)/man8
docdir  = $(prefix)/share/doc/dmidecode

INSTALL         := install
INSTALL_DATA    := $(INSTALL) -m 644
INSTALL_DIR     := $(INSTALL) -m 755 -d
INSTALL_PROGRAM := $(INSTALL) -m 755
RM              := rm -f

# BSD make provides $MACHINE, but GNU make doesn't
MACHINE ?= $(shell uname -m 2>/dev/null)

all : libdmi-cpux.so

#
# Programs
#

libdmi-cpux.so : dmidecode.o dmiopt.o dmioem.o util.o
	$(CC) -shared dmidecode.o dmiopt.o dmioem.o util.o -o $@

#
# Objects
#

dmidecode.o : dmidecode.c types.h util.h config.h dmidecode.h dmiopt.h dmioem.h libdmi.h
	$(CC) $(CFLAGS) $(LIBFLAGS) -c $< -o $@ -DCPUX

dmiopt.o : dmiopt.c config.h types.h util.h dmidecode.h dmiopt.h
	$(CC) $(CFLAGS) $(LIBFLAGS) -c $< -o $@

dmioem.o : dmioem.c types.h dmidecode.h dmioem.h
	$(CC) $(CFLAGS) $(LIBFLAGS) -c $< -o $@

util.o : util.c types.h util.h config.h
	$(CC) $(CFLAGS) $(LIBFLAGS) -c $< -o $@

#
# Commands
#

clean :
	$(RM) *.o libdmi-cpux.so
