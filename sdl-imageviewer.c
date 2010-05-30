//	imgv, a simple SDL-based image viewer for the Ben Nanonote
//	Version 0.2.1
//	Last edited by Fernando Carello <fcarello@libero.it> 2010-05-24
//
#include <stdlib.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>

#define TRUE 1
#define FALSE 0
#define SCREENWIDTH 320
#define SCREENHEIGHT 240
#define SCREENBPP 32
#define SMOOTHING_OFF	0
#define SMOOTHING_ON	1
#define PANSTEP 40
#define ZOOMSTEP 0.05

int main(int argc, char *argv[])
{
	SDL_Surface 		*screen 	= NULL,
				*scaled_img 	= NULL,
				*temp_img 	= NULL,
	 			*picture    	= NULL;
	SDL_Event 		event;
	SDL_Rect 		picturePortion,
				screenPortion;
	int 			imgWidth,
				imgHeight,
				smoothing	= SMOOTHING_ON,
				fPressed,
				zPressed,
				iPressed,
				oPressed,
				alreadyFit,
				pixelFit,
				lPressed, 
				rPressed, 	
				leftPressed, 
				rightPressed, 
				upPressed, 
				downPressed;
	char *			sFilename;
	char			sVersion[]	= "0.2.1";
	double			scale_x 	= 1.0, 
				scale_y 	= 1.0, 
				scale 		= 1.0;


	atexit(SDL_Quit);

	// Process command line
	if (argc != 2)
	{
		fprintf (stderr,  "\n"
			 " imgv v%s. Syntax: imgv <image file>\n\n"
			 " Hotkeys:\n"
			 " 'f' fit to screen\n"
			 " 'z' zoom at pixel level\n"
			 " 'i' zoom in  'o' zoom out\n"
			 " 'l' rotate left  'r' rotate right\n"
			 " 'arrows' pan  'ESC' quit\n\n", sVersion);
		exit (1);
	}
	sFilename = argv[1];

	// Initialize the SDL library 
	if (SDL_Init(SDL_INIT_VIDEO) < 0) 
	{
		fprintf(stderr,	"\n Couldn't initialize SDL: %s\n\n", SDL_GetError());
		exit(1);
	}

	// Load Picture
	temp_img = IMG_Load(sFilename);
	if (temp_img == (SDL_Surface *) (NULL)) 
	{
		fprintf(stderr, "\n Couldn't load image file %s: %s\n\n", sFilename, SDL_GetError());
		exit (1);
	}

	// Set video mode
	screen = SDL_SetVideoMode 
		(SCREENWIDTH, SCREENHEIGHT, SCREENBPP, SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWACCEL);
	if (screen == (SDL_Surface *) (NULL)) 
	{
		fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n\n", SCREENWIDTH, SCREENHEIGHT, SCREENBPP, SDL_GetError());
		exit(1);
	}
	
	// Can't stand the useless arrow... Ben has no pointing device
	SDL_ShowCursor(SDL_DISABLE);	

	// Convert picture in same format as video framebuffer, to optimize blit performances
	picture = SDL_DisplayFormat (temp_img);
	if (picture == (SDL_Surface *) (NULL)) 
	{
		fprintf(stderr, "\n Internal error from DisplayFormat\n\n");
		exit (1);
	}
	SDL_FreeSurface (temp_img);
	
	imgWidth 	= picture->w;
	imgHeight	= picture->h;

	// Starting point (default view): uniform scaling to best screen fit, keep aspect ratio
	scale_x		= (double) (SCREENWIDTH) / (double) (imgWidth);
	scale_y		= (double) (SCREENHEIGHT) / (double) (imgHeight);
	if (scale_y < scale_x)
		scale = scale_y;
	else
		scale = scale_x;
	scaled_img = zoomSurface (picture, scale, scale, SMOOTHING_ON);
	if (scaled_img == (SDL_Surface *) (NULL))
	{
		fprintf (stderr, "\n Error from zoomSurface()\n\n");
		exit (1);
	}
	
	screenPortion.x 	= 0;	// destination coordinates: origin
	screenPortion.y 	= 0;
	picturePortion.x	= 0;
	picturePortion.y	= 0;
	picturePortion.w	= (Uint16) imgWidth;
	picturePortion.h	= (Uint16) imgHeight;

	// off-cycle: first image drawing
	SDL_FillRect(screen, NULL, 0);	// draw background color (black)
	SDL_BlitSurface(scaled_img, &picturePortion, screen, &screenPortion); 
	SDL_Flip(screen);
	
	leftPressed 		= FALSE;
	rightPressed 		= FALSE;
	upPressed 		= FALSE;
	downPressed 		= FALSE;
	fPressed		= FALSE;
	zPressed		= FALSE;
	lPressed		= FALSE;
	rPressed		= FALSE;
	iPressed		= FALSE;
	oPressed		= FALSE;
	pixelFit		= FALSE;
	alreadyFit		= TRUE;

	while(1) 
	{
		if (SDL_WaitEvent( &event ))
		{
			// We only process SDL_KEYDOWN and SDL_KEYUP events 
			switch (event.type)
			{
		  		case SDL_KEYDOWN:
			  		switch (event.key.keysym.sym) 
					{
		  				case SDLK_LEFT:
			  				leftPressed = TRUE;
			  			break;
		  				case SDLK_RIGHT:
			  				rightPressed = TRUE;
			  			break;
		  				case SDLK_DOWN:
			  				downPressed = TRUE;
			  			break;
		  				case SDLK_f:
			  				fPressed = TRUE;
			  			break;
		  				case SDLK_z:
			  				zPressed = TRUE;
			  			break;
		  				case SDLK_i:
			  				iPressed = TRUE;
			  			break;
		  				case SDLK_o:
			  				oPressed = TRUE;
			  			break;
		  				case SDLK_l:
			  				lPressed = TRUE;
			  			break;
		  				case SDLK_r:
			  				rPressed = TRUE;
			  			break;
		  				case SDLK_UP:
			  				upPressed = TRUE;
			  			break;
		  				case SDLK_ESCAPE:
							if (picture)
								SDL_FreeSurface (picture);
							if (scaled_img)
								SDL_FreeSurface (scaled_img);
							if (screen)
								SDL_FreeSurface (screen);
			  				exit(0);
		  				default:
			  			break;
			  		}
			  	break;
		 		case SDL_KEYUP:
			  		switch (event.key.keysym.sym) 
					{
		  				case SDLK_LEFT:
			  				leftPressed = FALSE;
			  			break;
		  				case SDLK_RIGHT:
			  				rightPressed = FALSE;
			  			break;
		  				case SDLK_DOWN:
			  				downPressed = FALSE;
			  			break;
		  				case SDLK_UP:
			  				upPressed = FALSE;
			  			break;
		  				case SDLK_f:
			  				fPressed = FALSE;
			  			break;
		  				case SDLK_z:
			  				zPressed = FALSE;
			  			break;
		  				case SDLK_i:
			  				iPressed = FALSE;
			  			break;
		  				case SDLK_o:
			  				oPressed = FALSE;
			  			break;
		  				case SDLK_l:
			  				lPressed = FALSE;
			  			break;
		  				case SDLK_r:
			  				rPressed = FALSE;
			  			break;
		  				default:
			  			break;
			  		}
			  	break;
			} // end of switch (event.type)

			// Process commands
			if (fPressed && !alreadyFit)
			{	// Fit image to screen
				if (scaled_img != (SDL_Surface *) (NULL))
					SDL_FreeSurface (scaled_img);	// since zoomSurface() creates a new surface every time
				alreadyFit 	= TRUE;
				pixelFit   	= FALSE;
				scale_x		= (double) (SCREENWIDTH) / (double) (imgWidth);
				scale_y		= (double) (SCREENHEIGHT) / (double) (imgHeight);
				if (scale_y < scale_x)
					scale = scale_y;
				else
					scale = scale_x;
				picturePortion.x = 0;
				picturePortion.y = 0;
				picturePortion.w = (Uint16) imgWidth;
				picturePortion.h = (Uint16) imgHeight;
				scaled_img = zoomSurface (picture, scale, scale, smoothing);
				if (scaled_img == (SDL_Surface *) (NULL))
				{
					fprintf (stderr, "\n Error from zoomSurface()\n\n");
					exit (1);
				}
			}
			if (zPressed && !pixelFit)
			{	// Zoom at 1:1 (100% zoom / actual pixels)
				if (scaled_img != (SDL_Surface *) (NULL))
					SDL_FreeSurface (scaled_img);
				alreadyFit = FALSE;
				pixelFit   = TRUE;
				picturePortion.w = (Uint16) SCREENWIDTH;
				picturePortion.h = (Uint16) SCREENHEIGHT;
				scale 	   = 1.0;
				scaled_img = SDL_ConvertSurface (picture, picture->format, picture->flags);
				if (scaled_img == (SDL_Surface *) (NULL))
				{
					fprintf (stderr, "\n Error from ConvertSurface()\n\n");
					exit (1);
				}
			}
			if (iPressed)
			{	// Zoom in
				if (scaled_img != (SDL_Surface *) (NULL))
					SDL_FreeSurface (scaled_img);
				alreadyFit = FALSE;
				pixelFit   = FALSE;
				scale += ZOOMSTEP;
				picturePortion.w = (Uint16) ((double) (imgWidth) * scale);
				if ( (picturePortion.w - picturePortion.x) >= imgWidth )
					picturePortion.w = imgWidth - picturePortion.x;
				if (picturePortion.w < 1)
					picturePortion.w = 1;
				picturePortion.h = (Uint16) ((double) (imgHeight) * scale);
				if ( (picturePortion.h - picturePortion.y) >= imgHeight )
					picturePortion.h = imgHeight - picturePortion.y;
				if (picturePortion.h < 1)
					picturePortion.h = 1;
				scaled_img = zoomSurface (picture, scale, scale, smoothing);
				//scaled_img = rotozoomSurface (picture, 0, scale, SMOOTHING_ON);		
				
				if (scaled_img == (SDL_Surface *) (NULL))
				{
					fprintf (stderr, "\n Error from zoomSurface()\n\n");
					exit (1);
				}
			}
			if (oPressed)
			{	// Zoom out
				if (scaled_img != (SDL_Surface *) (NULL))
					SDL_FreeSurface (scaled_img);
				alreadyFit = FALSE;
				pixelFit   = FALSE;
				scale -= ZOOMSTEP;
				if (scale <= 0.0)
					scale = 0.01;
				picturePortion.w = (Uint16) ((double) (imgWidth) * scale);
				if ( (picturePortion.w - picturePortion.x) >= imgWidth)
					picturePortion.w = imgWidth - picturePortion.x;
				if (picturePortion.w < 1)
					picturePortion.w = 1;
				picturePortion.h = (Uint16) ((double) (imgHeight) * scale);
				if ( (picturePortion.h - picturePortion.y) >= imgHeight )
					picturePortion.h = imgHeight - picturePortion.y;
				if (picturePortion.h < 1)
					picturePortion.h = 1;
					
				scaled_img = zoomSurface (picture, scale, scale, smoothing);
				//scaled_img = rotozoomSurface (picture, 0, scale, SMOOTHING_ON);
				if (scaled_img == (SDL_Surface *) (NULL))
				{
					fprintf (stderr, "\n Error from zoomSurface()\n\n");
					exit (1);
				}
			}

			if (lPressed)
			{	// Rotate left 90°
				if (scaled_img != (SDL_Surface *) (NULL))
					temp_img   = scaled_img;
				else
				{
					fprintf (stderr, "\n Error: NULL scaled_img\n\n");
					exit (1);
				}		
				alreadyFit = FALSE;
				scaled_img = rotozoomSurface (temp_img, -90, 1.0, SMOOTHING_OFF);
				SDL_FreeSurface (temp_img);
				if (scaled_img == (SDL_Surface *) (NULL))
				{
					fprintf (stderr, "\n Error from rotozoomSurface()\n\n");
					exit (1);
				}		
			}
			if (rPressed)
			{	// Rotate right 90°
				if (scaled_img != (SDL_Surface *) (NULL))
					temp_img   = scaled_img;
				else
				{
					fprintf (stderr, "\n Error: NULL scaled_img\n\n");
					exit (1);
				}		
				alreadyFit = FALSE;
				scaled_img = rotozoomSurface (temp_img, 90, 1.0, SMOOTHING_OFF);
				SDL_FreeSurface (temp_img);
				if (scaled_img == (SDL_Surface *) (NULL))
				{
					fprintf (stderr, "\n Error from rotozoomSurface()\n\n");
					exit (1);
				}		
			}
			
			if (leftPressed) 
			{
				picturePortion.x -= PANSTEP;
				if (picturePortion.x < 0)
					picturePortion.x = 0;
				alreadyFit = FALSE;
			}
			if (rightPressed) 
			{
				picturePortion.x += PANSTEP;
				if (picturePortion.x >= imgWidth)
					picturePortion.x = (Sint16) (imgWidth - 1);
				alreadyFit = FALSE;
			}
			if (upPressed) 
			{
				picturePortion.y -= PANSTEP;
				if (picturePortion.y < 0)
					picturePortion.y = 0;
				alreadyFit = FALSE;
			}
			if (downPressed) 
			{
				picturePortion.y += PANSTEP;
				if (picturePortion.y >= imgHeight)
					picturePortion.y = (Sint16) (imgHeight - 1);
				alreadyFit = FALSE;
			}

			SDL_FillRect(screen, (SDL_Rect *) NULL, 0);	// draw background color (black)
			SDL_BlitSurface(scaled_img, &picturePortion, screen, &screenPortion); 
			SDL_Flip(screen);
		} // end of if(SDL_WaitEvent())
	}	// end of while(1)

	return 0;
}

