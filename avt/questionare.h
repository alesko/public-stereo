
#ifndef __QUESTIONARE_H__
#define __QUESTIONARE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* SDL and OpenGL */

#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>
#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

//#include "globals.h"

/* Setup our booleans */
//#define TRUE  1
//#define FALSE 0
#define true  1
#define false 0
typedef char bool;

/* Number of textures to load */
#define NUM_TEXTURES 2

/* This is our SDL surface */
//SDL_Surface *surface;

GLuint  base;                  /* Base Display List For The Font           */
GLuint  texture[NUM_TEXTURES]; /* Storage For Our Font Texture             */
//GLuint  loop;                  /* Generic Loop Variable                    */


void SDLinit(int width, int height);
void initTexture(GLuint* tex, int num);

// new window size or exposure
void reshape(int width, int height);
void idle(void);

void left(GLint w, GLint h, float s, float portion);
void right(GLint w, GLint h, float s, float portion);

void drawTexture(int width, int height, int device_width, int device_height, float s, GLuint* tex, unsigned char* data_left, unsigned char* data_right , float portion);
void drawOverlayTexture(int width, int height, int device_width, int device_height, float s, GLuint* tex );
void drawBlank(void);


// function to recover memory form our list of characters 
GLvoid KillFont( GLvoid ); //base

// function to load in bitmap as a GL texture 
int LoadGLTextures(); //texture

// function to build our font list 
GLvoid BuildFont( GLvoid ); // texture

/* general OpenGL initialization function */
int initGLfont( GLvoid );

/* Function to print the string */
GLvoid glPrint( GLint w, GLint h, GLint x, GLint y, char *string, int set ); //texture
GLvoid glPrintStereo( GLint w, GLint h, GLint x, GLint y, char *string, int set );

/* general OpenGL initialization function */
int initGLFont( GLvoid );

/* Here goes our drawing code */
int drawGLQuestion( GLvoid ); 

#ifdef __cplusplus
}
#endif

#endif
