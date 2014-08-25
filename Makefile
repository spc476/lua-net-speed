
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

