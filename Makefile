
CC      = gcc -std=c99
CFLAGS  = -g -Wall -Wextra -pedantic
LDFLAGS = 
LDLIBS  = -llua -ldl -lm

.PHONE : all clean

all : blast suck

clean :
	$(RM) -r *~ blast suck

blast :
suck :