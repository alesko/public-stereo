#
#
#

#TARGET = liblabjackusb.so.2.0.3
#TARGET = 
#DESTINATION = /usr/local/lib

#HEADER = globals.h
#HEADER_DESTINATION = /usr/local/include

CFLAGS +=  -Wall  -O2
LIBFLAGS = -ldc1394 -lGL -L/usr/include -L/usr/X11R6/lib -lX11 -lXext -lXv -lXi  -lglut  -lpthread #-lXmu
SDLFLAGS= `sdl-config --cflags --libs`
CC = gcc
NV = /usr/local/cuda/bin/nvcc

%.o: %.c
	$(CC) $(CFLAGS) -c $<

#all: test.c camera.c definitions.c testSDL.c
#	$(CC) -o test test.c camera.c definitions.c -I. $(LIBFLAGS)
#	$(CC) -o testSDL testSDL.c camera.c definitions.c -I. $(LIBFLAGS) $(SDLFLAGS)

cuda: conv.cu
	$(NV) -c *cu
	$(CC) *o -o cuda_multiview $(LIBFLAGS)

all: avt_singleview.c dc1394_multiview.c 
#	$(CC) -o test test.c camera.c definitions.c -I. $(LIBFLAGS)
	$(CC) -o avt_singleview-new avt_singleview.c -I. $(LIBFLAGS)
	$(CC) -o dc1394_multiview dc1394_multiview.c -I. $(LIBFLAGS)

gl: stereoviewGL.c
	$(CC) -o stereoviewGL stereoviewGL.c -I. $(LIBFLAGS) $(SDLFLAGS)

text: glutfonts.c
	$(CC) -o glutfonts glutfonts.c -I. $(LIBFLAGS) $(SDLFLAGS)