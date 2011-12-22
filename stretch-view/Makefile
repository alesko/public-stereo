#
# Makefile for liblabjackusb
#
#

#TARGET = liblabjackusb.so.2.0.3
TARGET = test
#DESTINATION = /usr/local/lib

HEADER = globals.h
#HEADER_DESTINATION = /usr/local/include

CFLAGS +=  -Wall 
LIBFLAGS = -lraw1394 -ldc1394 -lGL -lglut  -lpthread
SDLFLAGS=`sdl-config --cflags --libs`
CC = gcc

%.o: %.c
	$(CC) $(CFLAGS) -c $<

#all: test.c camera.c definitions.c testSDL.c
#	$(CC) -o test test.c camera.c definitions.c -I. $(LIBFLAGS)
#	$(CC) -o testSDL testSDL.c camera.c definitions.c -I. $(LIBFLAGS) $(SDLFLAGS)

all: main.c
#	$(CC) -o test test.c camera.c definitions.c -I. $(LIBFLAGS)
	$(CC) -o testSDL main.c -I. $(LIBFLAGS) $(SDLFLAGS)

#	$(CC) -o texture_sub  texture_sub.c $(LIBFLAGS)
#gcc -Wall -lglut -ldc1394_control -lraw1394 -lGL -lGLU texture_sub.c -o texture_sub

#$(TARGET): labjackusb.o $(HEADER)
#	$(CC) -shared -Wl,-soname,liblabjackusb.so -o $(TARGET) labjackusb.o $(LIBFLAGS)

#install: $(TARGET)
#	test -z $(DESTINATION) || mkdir -p $(DESTINATION)
#	install $(TARGET) $(DESTINATION)
#	test -z $(HEADER_DESTINATION) || mkdir -p $(HEADER_DESTINATION)
#	install $(HEADER) $(HEADER_DESTINATION)
#	ldconfig $(DESTINATION)

#clean:
#	rm -f $(TARGET) *.o *~


