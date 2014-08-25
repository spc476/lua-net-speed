#########################################################################
#
# Copyright 2014 by Sean Conner.  All Rights Reserved.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, see <http://www.gnu.org/licenses/>.
#
# Comments, questions and criticisms can be sent to: sean@conman.org
#
########################################################################

CC      = gcc -std=c99
CFLAGS  = -g -Wall -Wextra -pedantic
LDFLAGS = 
LDLIBS  = -llua -ldl -lm

.PHONE : all clean

all : blast suck buffer.so net.so

clean :
	$(RM) -r *~ blast suck *.so

blast :
suck :
buffer.so : buffer.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

net.so : net.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

