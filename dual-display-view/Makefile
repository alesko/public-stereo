# LINUX
CFLAGS = -g -I/usr/include/opencv -DLINUX
LDFLAGS = -lcxcore -lhighgui -lm 
CPP = g++

SRC = stereoview_video.cpp
OBJ = stereoview_video.o


all: $(SRC)
	g++ -c stereoview_video.cpp -o stereoview_video.o -g -I/usr/include/opencv -DLINUX
	g++ stereoview_video.o $(LDFLAGS) -o stereoview_video

clean:
	rm -f stereoview_video *.o 