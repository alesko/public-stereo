#include "questionare.h"


//static 
void SDLinit(int width, int height)
{

  glEnable( GL_TEXTURE_2D );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glViewport( 0, 0, width, height );
  glClear( GL_COLOR_BUFFER_BIT );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();


}



void initTexture(GLuint* tex, int num) //int w, int h)
{

  int i;
  
  //glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glDisable(GL_BLEND);
  //glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);

  //glEnable(GL_TEXTURE_2D);

  for( i = 0; i<2;i++){
    glGenTextures(1, &tex[i]);
    glBindTexture(GL_TEXTURE_2D, tex[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glClear( GL_COLOR_BUFFER_BIT );
  }

}

//static 
void idle(void)
{
  usleep(1000); 
}

// new window size or exposure
//static 
void reshape(int width, int height)
{
  GLfloat h = (GLfloat) height / (GLfloat) width;

  glViewport(0, 0, (GLint) width, (GLint) height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -40.0);
}

void left(GLint w, GLint h, float s, float portion)
{
  GLfloat p0,p1;
  GLfloat border = 0.0; // part of image
  GLfloat borderpx = 0.0*w; // part of image

  if( s > 1.0)
    {
      p0 = (s -1.0)/2.0 + border;
      p1 = 1.0 - (s -1.0)/2.0 - border;
      // Draw a textured quad	
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex2f((0.0f), 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex2f((GLfloat)w/2, 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)w/2, (GLfloat)h );
      glTexCoord2f(0.0f, 1.0f); glVertex2f((0.0f), (GLfloat)h );
      glEnd();
      glBegin(GL_QUADS);
      glTexCoord2f(p0, portion); glVertex2f(0.0f+borderpx, (GLfloat)(portion*h) );
      glTexCoord2f(p1, portion); glVertex2f((GLfloat)(w/2-borderpx), (GLfloat)(portion*h) );
      glTexCoord2f(p1, 1.0f); glVertex2f((GLfloat)(w/2-borderpx), (GLfloat)h );
      glTexCoord2f(p0, 1.0f); glVertex2f(0.0f+borderpx, (GLfloat)h );
      glEnd();

    }
  if( s <= 1.0)
    {
      float pix = s*w/2.0;
      p0 = w/2.0-pix; 
      p1 = pix; 
      // Draw a textured quad
      /*glBegin(GL_QUADS);	
      glTexCoord2f(0.0f, 0.0f); glVertex2f((GLfloat)p0, 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex2f((GLfloat)p1, 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)p1, (GLfloat)h );
      glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)p0, (GLfloat)h );
      glEnd();*/
      glBegin(GL_QUADS);	
      glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex2f(w/2, 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex2f(w/2, (GLfloat)h );
      glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, (GLfloat)h );
      glEnd();

      // Remove this segemet to remove black borders
      glBegin(GL_QUADS); 
      glColor3f(0.0,0.0,0.0);     
      glVertex2f((0.0f), (portion*h));
      glVertex2f((GLfloat)w/2, (portion*h));
      glVertex2f((GLfloat)w/2, (GLfloat)h );
      glVertex2f((0.0f), (GLfloat)h );
      glColor3f(1.0,1.0,1.0);
      glEnd();


      glBegin(GL_QUADS);	
      glTexCoord2f(0.0f, portion); glVertex2f((GLfloat)p0, (GLfloat)(portion*h));
      glTexCoord2f(1.0f, portion); glVertex2f((GLfloat)p1, (GLfloat)(portion*h));
      glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)p1, (GLfloat)h );
      glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)p0, (GLfloat)h );
      glEnd();

      
    }
  
}

void right(GLint w, GLint h, float s, float portion)
{
  GLfloat p0,p1;

  if( s > 1.0)
    {
      p0 = (s -1.0)/2.0;
      p1 = 1.0 - (s -1.0)/2.0;
      // Draw a textured quad	
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 0.0f); glVertex2f((GLfloat)w/2, 0.0f);
      glTexCoord2f(1.0, 0.0f); glVertex2f((GLfloat)w, 0.0f);
      glTexCoord2f(1.0, 1.0f); glVertex2f((GLfloat)w, (GLfloat)h );
      glTexCoord2f(0.0, 1.0f); glVertex2f((GLfloat)w/2, (GLfloat)h );
      glEnd();
      glBegin(GL_QUADS);
      glTexCoord2f(p0, portion); glVertex2f((GLfloat)w/2, (GLfloat)(h*portion));
      glTexCoord2f(p1, portion); glVertex2f((GLfloat)w, (GLfloat)(h*portion));
      glTexCoord2f(p1, 1.0f); glVertex2f((GLfloat)w,  (GLfloat)h );
      glTexCoord2f(p0, 1.0f); glVertex2f((GLfloat)w/2, (GLfloat)h );
      glEnd();
    }
  if( s <= 1.0)
    {
      float pix = s*w/2.0;
      p0 = (GLfloat)w/2+w/2.0-pix; 
      p1 = (GLfloat)w/2+pix; 
      // Draw a textured quad
      /*glBegin(GL_QUADS);	
      glTexCoord2f(0.0f, 0.0f); glVertex2f((GLfloat)p0, 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex2f((GLfloat)p1, 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)p1, (GLfloat)h );
      glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)p0, (GLfloat)h );
      glEnd();*/
      glBegin(GL_QUADS);	
      glTexCoord2f(0.0f, 0.0f); glVertex2f((GLfloat)w/2, 0.0f);
      glTexCoord2f(1.0f, 0.0f); glVertex2f((GLfloat)w, 0.0f);
      glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)w, (GLfloat)h );
      glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)w/2, (GLfloat)h );
      glEnd();

      // Remove this segemet to remove black borders
      glBegin(GL_QUADS); 
      glColor3f(0.0,0.0,0.0);     
      glVertex2f((GLfloat)w/2, (portion*h));
      glVertex2f((GLfloat)w, (portion*h));
      glVertex2f((GLfloat)w, (GLfloat)h );
      glVertex2f((GLfloat)w/2, (GLfloat)h );
      glColor3f(1.0,1.0,1.0);
      glEnd();

      glBegin(GL_QUADS);	
      glTexCoord2f(0.0f, portion); glVertex2f((GLfloat)p0, (GLfloat)(h*portion));
      glTexCoord2f(1.0f, portion); glVertex2f((GLfloat)p1, (GLfloat)(h*portion));
      glTexCoord2f(1.0f, 1.0f); glVertex2f((GLfloat)p1,  (GLfloat)h);
      glTexCoord2f(0.0f, 1.0f); glVertex2f((GLfloat)p0, (GLfloat)h );
      glEnd();
    }
}


//static 
void drawTexture(int width, int height, int device_width, int device_height,float s, GLuint* tex, unsigned char* data_left, unsigned char* data_right, float portion)
{
  //int i;
  //int frame_ready = 1;
  //GLfloat p0,p1;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  
  // Set Projection Matrix
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
  
  // Switch to Model View Matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glBindTexture( GL_TEXTURE_2D, tex[0] );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, device_width,device_height,0, GL_RGB, GL_UNSIGNED_BYTE, data_left); //frames[i]->image);
  left(width,height, s, portion);
  glBindTexture( GL_TEXTURE_2D, tex[1] );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, device_width,device_height,0, GL_RGB, GL_UNSIGNED_BYTE, data_right); //frames[i]->image);
  right(width,height, s, portion);

}

void drawOverlayTexture(int width, int height, int device_width, int device_height, float s, GLuint* tex )
{

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  
  // Set Projection Matrix
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
  
  // Switch to Model View Matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glBindTexture( GL_TEXTURE_2D, tex[0] );
  left(width,height, s, 0.0);
  glBindTexture( GL_TEXTURE_2D, tex[1] );
  right(width,height, s, 0.0);

}


void drawBlank(void)
{

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}


/*******************************************************************************************/



// function to recover memory form our list of characters 
GLvoid KillFont( GLvoid )
{
  glDeleteLists( base, 256 ); // Delete All 256 Display Lists 

  return;
}

// function to load in bitmap as a GL texture 
int LoadGLTextures()
{
  // Status indicator 
  //int Status = FALSE;
  bool status = false;

  // Create storage space for the texture 
  SDL_Surface *TextureImage[2];

  // Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit 
  if ( ( TextureImage[0] = SDL_LoadBMP( "font.bmp" ) ) && 
       ( TextureImage[1] = SDL_LoadBMP( "bumps.bmp" ) ) )
    {
	
      // Set the status to true 
      //Status = TRUE;
      status = true;

      // Create The Texture
      glGenTextures( NUM_TEXTURES, &texture[0] );

      // Load in texture 1
      // Typical Texture Generation Using Data From The Bitmap 
      glBindTexture( GL_TEXTURE_2D, texture[0] );

      // Generate The Texture
      glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[0]->w,
		    TextureImage[0]->h, 0, GL_BGR,
		    GL_UNSIGNED_BYTE, TextureImage[0]->pixels );
	    
      // Nearest Filtering
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

      // Load in texture 2
      // Typical Texture Generation Using Data From The Bitmap 
      glBindTexture( GL_TEXTURE_2D, texture[1] );

      // Linear Filtering 
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

      // Generate The Texture 
      glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[1]->w,
		    TextureImage[1]->h, 0, GL_BGR,
		    GL_UNSIGNED_BYTE, TextureImage[1]->pixels );
    }

  // Free up any memory we may have used 
  if ( TextureImage[0] )
    SDL_FreeSurface( TextureImage[0] );
  if ( TextureImage[1] )
    SDL_FreeSurface( TextureImage[1] );

  //return Status;
  return (int)status;
}

// function to build our font list 
GLvoid BuildFont( GLvoid )
{
  GLuint loop; /* Loop variable               */
  float cx;    /* Holds Our X Character Coord */
  float cy;    /* Holds Our Y Character Coord */

  /* Creating 256 Display List */
  base  = glGenLists( 256 );
  /* Select Our Font Texture */
  glBindTexture( GL_TEXTURE_2D, texture[0] );

  /* Loop Through All 256 Lists */
  for ( loop = 0; loop < 256; loop++ )
    {
      /* NOTE:
       *  BMPs are stored with the top-leftmost pixel being the
       * last byte and the bottom-rightmost pixel being the first
       * byte. So an image that is displayed as
       *    1 0
       *    0 0
       * is represented data-wise like
       *    0 0
       *    0 1
       * And because SDL_LoadBMP loads the raw data without
       * translating to how it is thought of when viewed we need
       * to start at the bottom-right corner of the data and work
       * backwards to get everything properly. So the below code has
       * been modified to reflect this. Examine how this is done and
       * how the original tutorial is done to grasp the differences.
       *
       * As a side note BMPs are also stored as BGR instead of RGB
       * and that is why we load the texture using GL_BGR. It's
       * bass-ackwards I know but whattaya gonna do?
       */

      /* X Position Of Current Character */
      cx = 1 - ( float )( loop % 16 ) / 16.0f;
      /* Y Position Of Current Character */
      cy = 1 - ( float )( loop / 16 ) / 16.0f;

      /* Start Building A List */
      glNewList( base + ( 255 - loop ), GL_COMPILE );
      /* Use A Quad For Each Character */
      glBegin( GL_QUADS );
      /* Texture Coord (Bottom Left) */
      glTexCoord2f( cx - 0.0625, cy );
      /* Vertex Coord (Bottom Left) */
      glVertex2i( 0, 0 );

      /* Texture Coord (Bottom Right) */
      glTexCoord2f( cx, cy );
      /* Vertex Coord (Bottom Right) */
      glVertex2i( 16, 0 );

      /* Texture Coord (Top Right) */
      glTexCoord2f( cx, cy - 0.0625f );
      /* Vertex Coord (Top Right) */
      glVertex2i( 16, 16 );

      /* Texture Coord (Top Left) */
      glTexCoord2f( cx - 0.0625f, cy - 0.0625f);
      /* Vertex Coord (Top Left) */
      glVertex2i( 0, 16 );
      glEnd( );

      /* Move To The Left Of The Character */
      glTranslated( 10, 0, 0 );
      glEndList( );
    }
}

/* general OpenGL initialization function */
int initGLfont( GLvoid )
{

  /* Load in the textures */
  if ( !LoadGLTextures( ) )
    return (int)false;

    /* Build our font list */
    BuildFont( );

    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* The Type Of Depth Test To Do */
    glDepthFunc( GL_LEQUAL );

    /* Select The Type Of Blending */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    /* Enable 2D Texture Mapping */
    glEnable( GL_TEXTURE_2D );
    
    return (int)true;
}

/* Function to print the string */
GLvoid glPrint( GLint w, GLint h, GLint x, GLint y, char *string, int set )
{
    if ( set > 1 )
	set = 1;

    //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
 
    /* Re-enable Blending */
    glEnable( GL_BLEND );

    /* Select our texture */
    glBindTexture( GL_TEXTURE_2D, texture[0] );

    /* Disable depth testing */
    glDisable( GL_DEPTH_TEST );

    /* Select The Projection Matrix */
    glMatrixMode( GL_PROJECTION );
    /* Store The Projection Matrix */
    glPushMatrix( );

    /* Reset The Projection Matrix */
    glLoadIdentity( );
    /* Set Up An Ortho Screen */
    glOrtho( 0, w, 0, h, -1, 1 );

    /* Select The Modelview Matrix */
    glMatrixMode( GL_MODELVIEW );
    /* Stor the Modelview Matrix */
    glPushMatrix( );
    /* Reset The Modelview Matrix */
    glLoadIdentity( );

    /* Position The Text (0,0 - Bottom Left) */
    glTranslated( x, y, 0 );

    /* Choose The Font Set (0 or 1) */
    glListBase( base - 32 + ( 128 * set ) );

    /* Write The Text To The Screen */
    glCallLists( strlen( string ), GL_BYTE, string );

    /* Select The Projection Matrix */
    glMatrixMode( GL_PROJECTION );
    /* Restore The Old Projection Matrix */
    glPopMatrix( );

    /* Select the Modelview Matrix */
    glMatrixMode( GL_MODELVIEW );
    /* Restore the Old Projection Matrix */
    glPopMatrix( );

    /* Re-enable Depth Testing */
    glEnable( GL_DEPTH_TEST );
}

/* general OpenGL initialization function */
int initGLFont( GLvoid )
{

  /* Load in the textures */
  if ( !LoadGLTextures( ) )
    return (int)false;

    /* Build our font list */
    BuildFont( );

    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* The Type Of Depth Test To Do */
    glDepthFunc( GL_LEQUAL );

    /* Select The Type Of Blending */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    /* Enable 2D Texture Mapping */
    glEnable( GL_TEXTURE_2D );
    
    return (int)true;
}

/* Function to print the string */
GLvoid glPrintStereo( GLint w, GLint h, GLint x, GLint y, char *string, int set )
{

  //printf("w %d, h %d, x %d, y %d\n",w, h, x,y);
  glPrint(w, h, x, y, string, set );
  glPrint(w, h, (x+w/2), y, string, set );

}

/* Here goes our drawing code */
int drawGLQuestion( GLvoid )
{
    /* These are to calculate our fps */
    //static GLint T0     = 0;
    //static GLint Frames = 0;

    /* Clear The Screen And The Depth Buffer */
    
  //
  //glColor3f( 1.0f , 1.0f , 1.0f );
  //glPrint((int) 640, (int) 480,  ( int ) (640/2),( int ) (480/2), "Is this question visibile and white?", 0 );

    /* Draw it to the screen */
    //SDL_GL_SwapBuffers( );

    /* Gather our frames per second */
    /*Frames++;
    {
	GLint t = SDL_GetTicks();
	if (t - T0 >= 5000) {
	    GLfloat seconds = (t - T0) / 1000.0;
	    GLfloat fps = Frames / seconds;
	    printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
	    T0 = t;
	    Frames = 0;
	}
    }
    usleep(10000);*/

    return (int)true;
}
