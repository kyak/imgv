//	imgv, a simple SDL-based image viewer for the Ben Nanonote
//	Version 0.3.0
//	Last edited by Fernando Carello <fcarello@libero.it> 2010-05-24
//	Last edited by Niels Kummerfeldt <niels.kummerfeldt@tuhh.de> 2010-10-19
//
#include <stdlib.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_ttf.h>

#define TRUE 1
#define FALSE 0
#define SCREENWIDTH 320
#define SCREENHEIGHT 240
#define SCREENBPP 32
#define SMOOTHING_OFF   0
#define SMOOTHING_ON    1
#define PANSTEP 40
#define ZOOMSTEP 1.2
#define SLIDESHOWTIMEOUT 1000 * 5
#define VERSION "0.3.0"

void quit()
{
    TTF_Quit();
    SDL_Quit();

    exit(1);
}

SDL_Surface *initScreen()
{
    // Initialize the SDL library 
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) 
    {
        fprintf(stderr,	"\n Couldn't initialize SDL: %s\n\n", SDL_GetError());
        quit();
    }

    // Set video mode
    SDL_Surface *screen = SDL_SetVideoMode(SCREENWIDTH, SCREENHEIGHT, SCREENBPP,
                                           SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWACCEL);
    if (screen == (SDL_Surface *) (NULL)) 
    {
        fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n\n", SCREENWIDTH, SCREENHEIGHT, SCREENBPP, SDL_GetError());
        quit();
    }

    // Can't stand the useless arrow... Ben has no pointing device
    SDL_ShowCursor(SDL_DISABLE);	

    TTF_Init();

    return screen;
}

SDL_Surface *loadImage(char *filename)
{
    // Load Picture
    SDL_Surface *tmp = IMG_Load(filename);
    if (tmp == (SDL_Surface *) (NULL)) 
    {
        fprintf(stderr, "\n Couldn't load image file %s: %s\n\n", filename, SDL_GetError());
        quit();
    }

    // Auto rotate image to fit screen
    //if (tmp->w > SCREENWIDTH || tmp->h > SCREENHEIGHT) {
    //    if (tmp->h > tmp->w * 1.1) {
    //        SDL_Surface *t = rotateSurface90Degrees(tmp, 3);
    //        SDL_FreeSurface(tmp);
    //        tmp = t;
    //    }
    //}

    // Convert picture in same format as video framebuffer, to optimize blit performances
    SDL_Surface *picture = SDL_DisplayFormat(tmp);
    if (picture == (SDL_Surface *) (NULL)) 
    {
        fprintf(stderr, "\n Internal error from DisplayFormat\n\n");
        quit();
    }
    SDL_FreeSurface(tmp);

    return picture;
}

void pan(SDL_Surface *image, SDL_Rect *pos, int dx, int dy)
{
    if (image->w > SCREENWIDTH) {
        pos->x += dx;
        if (pos->x < 0) {
            pos->x = 0;
        }
        if (pos->x >= image->w - SCREENWIDTH) {
            pos->x = (Sint16) (image->w - SCREENWIDTH);
        }
    } else {
        pos->x = 0;
    }
    if (image->h > SCREENHEIGHT) {
        pos->y += dy;
        if (pos->y < 0) {
            pos->y = 0;
        }
        if (pos->y >= image->h - SCREENHEIGHT) {
            pos->y = (Sint16) (image->h - SCREENHEIGHT);
        }
    } else {
        pos->y = 0;
    }
}

SDL_Surface *zoomIn(SDL_Surface *image, SDL_Rect *pos, double *scale)
{
    *scale *= ZOOMSTEP;

    SDL_Surface *result = zoomSurface(image, *scale, *scale, SMOOTHING_ON);
    if (result == (SDL_Surface *) (NULL))
    {
        fprintf(stderr, "\n Error from zoomSurface()\n\n");
        quit();
    }

    pos->x *= ZOOMSTEP;
    int dx = SCREENWIDTH * (ZOOMSTEP-1) * 0.5;
    pos->y *= ZOOMSTEP;
    int dy = SCREENHEIGHT * (ZOOMSTEP-1) * 0.5;
    pan(result, pos, dx, dy);

    return result;
}

SDL_Surface *zoomOut(SDL_Surface *image, SDL_Rect *pos, double *scale)
{
    *scale /= ZOOMSTEP;

    SDL_Surface *result = zoomSurface(image, *scale, *scale, SMOOTHING_ON);
    if (result == (SDL_Surface *) (NULL))
    {
        fprintf(stderr, "\n Error from zoomSurface()\n\n");
        quit();
    }

    pos->x += SCREENWIDTH * (1-ZOOMSTEP) * 0.5;
    pos->x /= ZOOMSTEP;
    pos->y += SCREENHEIGHT * (1-ZOOMSTEP) * 0.5;
    pos->y /= ZOOMSTEP;
    pan(result, pos, 0, 0);

    return result;
}

SDL_Surface *zoomFit(SDL_Surface *image, SDL_Rect *pos, double *scale)
{
    pos->x = 0;
    pos->y = 0;
    double scale_x = (double) (SCREENWIDTH) / (double) (image->w);
    double scale_y = (double) (SCREENHEIGHT) / (double) (image->h);
    if (scale_y < scale_x) {
        *scale = scale_y;
    } else {
        *scale = scale_x;
    }

    SDL_Surface *result = zoomSurface(image, *scale, *scale, SMOOTHING_ON);
    if (result == (SDL_Surface *) (NULL))
    {
        fprintf(stderr, "\n Error from zoomSurface()\n\n");
        quit();
    }

    return result;
}

SDL_Surface *zoom100(SDL_Surface *image, SDL_Rect *pos, double *scale)
{
    SDL_Surface *result = SDL_ConvertSurface(image, image->format, image->flags);
    if (result == (SDL_Surface *) (NULL))
    {
        fprintf(stderr, "\n Error from ConvertSurface()\n\n");
        quit();
    }

    if (*scale < 1.0) {
        pos->x /= *scale;
        pos->y /= *scale;
        pos->x -= SCREENWIDTH * (1-(1 / *scale)) * 0.5;
        pos->y -= SCREENHEIGHT * (1-(1 / *scale)) * 0.5;
    } else {
        pos->x += SCREENWIDTH * (1-*scale) * 0.5;
        pos->y += SCREENHEIGHT * (1-*scale) * 0.5;
        pos->x /= *scale;
        pos->y /= *scale;
    }
    pan(result, pos, 0, 0);
    *scale = 1.0;

    return result;
}

SDL_Surface *drawFileName(char *filename, TTF_Font *font, int slideShow)
{
    if(font) {
        SDL_Color foregroundColor = { 0, 0, 0, 0 }; 
        SDL_Color backgroundColor = { 200, 200, 200, 0 };

        char text[strlen(filename)+4];
        strcpy(text, filename);
        if (slideShow) {
            strcat(text, " >>");
        }
        return TTF_RenderText_Shaded(font, text, foregroundColor, backgroundColor);
    }
    return NULL;
}

void drawImage(SDL_Surface *image, SDL_Rect *pos, SDL_Surface *screen, SDL_Surface *filename)
{
    SDL_FillRect(screen, (SDL_Rect *) NULL, 0);	// draw background color (black)

    SDL_Rect screenPos;
    if (image->w < SCREENWIDTH) {
        screenPos.x = (SCREENWIDTH - image->w) / 2;
    } else {
        screenPos.x = 0;
    }
    if (image->h < SCREENHEIGHT) {
        screenPos.y = (SCREENHEIGHT - image->h) / 2;
    } else {
        screenPos.y = 0;
    }
    SDL_BlitSurface(image, pos, screen, &screenPos); 

    if(filename) {
        SDL_Rect textLocation = { 0, 0, 0, 0 };
        if (filename->w > SCREENWIDTH) {
            textLocation.x = SCREENWIDTH - filename->w;
        }
        SDL_BlitSurface(filename, NULL, screen, &textLocation);
    }

    SDL_Flip(screen);
}

Uint32 timerCallback(Uint32 interval, void *param)
{
    param = NULL;
    SDL_Event event;
    SDL_KeyboardEvent keyEvent;

    keyEvent.type = SDL_KEYDOWN;
    keyEvent.keysym.unicode = 0;
    keyEvent.keysym.scancode = 0;
    keyEvent.keysym.mod = 0;
    keyEvent.keysym.sym = SDLK_n;

    event.type = SDL_KEYDOWN;
    event.key = keyEvent;

    SDL_PushEvent(&event);

    return interval;
}

int main(int argc, char *argv[])
{
    SDL_Surface *screen      = NULL,
                *image       = NULL,
                *scaledImage = NULL,
                *name        = NULL;
    SDL_Rect     picturePortion;
    TTF_Font    *font = NULL;
    double       scale = 1.0;
    int          currentImageNumber = 1,
                 showFileName = TRUE,
                 runSlideShow = FALSE,
                 isRunning = TRUE;
    SDL_TimerID  slideShowTimer = 0;

    // Process command line
    if (argc < 2) {
        fprintf(stderr,  "\n"
            " imgv v%s. Syntax: imgv <image files>\n\n"
            " Hotkeys:\n"
            " 'f' fit to screen\n"
            " 'z' zoom at pixel level\n"
            " 'i' zoom in  'o' zoom out\n"
            " 'l' rotate left  'r' rotate right\n"
            " 'n' next image  'p' previous image\n"
            " 'd' show / hide file name\n"
            " 's' start / stop slide show\n"
            " 'arrows' pan  'ESC' quit\n\n", VERSION);
        exit(0);
    }

    screen = initScreen();

    font = TTF_OpenFont("font.ttf", 11);
    if (font == (TTF_Font *) (NULL)) {
        font = TTF_OpenFont("/usr/share/imgv/font.ttf", 11);
    }
    if (font == (TTF_Font *) (NULL)) {
        font = TTF_OpenFont("/usr/share/fonts/ttf-dejavu/DejaVuSans.ttf", 11);
    }

    picturePortion.w = SCREENWIDTH;
    picturePortion.h = SCREENHEIGHT;

    image = loadImage(argv[1]);
    if (image->w < SCREENWIDTH && image->h < SCREENHEIGHT) {
        scaledImage = zoom100(image, &picturePortion, &scale);
    } else {
        scaledImage = zoomFit(image, &picturePortion, &scale);
    }
    name = drawFileName(argv[currentImageNumber], font, runSlideShow);
    drawImage(scaledImage, &picturePortion, screen, name);

    do {
        SDL_Event event;
        if (SDL_WaitEvent(&event) && event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT: // PAN LEFT
                    pan(scaledImage, &picturePortion, -PANSTEP, 0);
	                break;
                case SDLK_RIGHT: // PAN RIGHT
                    pan(scaledImage, &picturePortion, PANSTEP, 0);
                    break;
                case SDLK_UP: // PAN UP
                    pan(scaledImage, &picturePortion, 0, -PANSTEP);
                    break;
                case SDLK_DOWN: // PAN DOWN
                    pan(scaledImage, &picturePortion, 0, PANSTEP);
                    break;
                case SDLK_i: // ZOOM IN
                    SDL_FreeSurface(scaledImage);
                    scaledImage = zoomIn(image, &picturePortion, &scale);
                    break;
                case SDLK_o: // ZOOM OUT
                    SDL_FreeSurface(scaledImage);
                    scaledImage = zoomOut(image, &picturePortion, &scale);
                    break;
                case SDLK_f: // ZOOM TO FIT SCREEN
                    SDL_FreeSurface(scaledImage);
                    scaledImage = zoomFit(image, &picturePortion, &scale);
                    break;
                case SDLK_z: // ZOOM TO ORIGINAL SIZE
                    SDL_FreeSurface(scaledImage);
                    scaledImage = zoom100(image, &picturePortion, &scale);
                    break;
                case SDLK_l: // ROTATE LEFT
                    {
                        SDL_FreeSurface(scaledImage);
                        SDL_Surface *tmp = rotateSurface90Degrees(image, 3);
                        SDL_FreeSurface(image);
                        image = tmp;
                        scaledImage = zoomSurface(image, scale, scale, SMOOTHING_ON);
                        int x = picturePortion.x;
                        picturePortion.x = picturePortion.y + SCREENHEIGHT/2 - SCREENWIDTH/2;
                        picturePortion.y = scaledImage->h - x - SCREENHEIGHT/2 - SCREENWIDTH/2;
                        pan(scaledImage, &picturePortion, 0, 0);
                    }
                    break;
                case SDLK_r: // ROTATE RIGHT
                    {
                        SDL_FreeSurface(scaledImage);
                        SDL_Surface *tmp = rotateSurface90Degrees(image, 1);
                        SDL_FreeSurface(image);
                        image = tmp;
                        scaledImage = zoomSurface(image, scale, scale, SMOOTHING_ON);
                        int x = picturePortion.x;
                        picturePortion.x = scaledImage->w - picturePortion.y - SCREENWIDTH/2
                                           - SCREENHEIGHT/2;
                        picturePortion.y = x + SCREENWIDTH/2 - SCREENHEIGHT/2;
                        pan(scaledImage, &picturePortion, 0, 0);
                    }
                    break;
                case SDLK_n: // NEXT IMAGE
                    if (currentImageNumber < argc-1) {
                        ++currentImageNumber;

                        SDL_FreeSurface(image);
                        SDL_FreeSurface(scaledImage);
                        SDL_FreeSurface(name);

                        image = loadImage(argv[currentImageNumber]);
                        if (image->w < SCREENWIDTH && image->h < SCREENHEIGHT) {
                            scaledImage = zoom100(image, &picturePortion, &scale);
                        } else {
                            scaledImage = zoomFit(image, &picturePortion, &scale);
                        }
                        name = drawFileName(argv[currentImageNumber], font, runSlideShow);
                    } else {
                        if (runSlideShow) {
                            SDL_RemoveTimer(slideShowTimer);
                            runSlideShow = FALSE;
                            name = drawFileName(argv[currentImageNumber], font, runSlideShow);
                        }
                    }
                    break;
                case SDLK_p: // PREVIOUS IMAGE
                    if (currentImageNumber > 1) {
                        --currentImageNumber;

                        SDL_FreeSurface(image);
                        SDL_FreeSurface(scaledImage);
                        SDL_FreeSurface(name);

                        image = loadImage(argv[currentImageNumber]);
                        if (image->w < SCREENWIDTH && image->h < SCREENHEIGHT) {
                            scaledImage = zoom100(image, &picturePortion, &scale);
                        } else {
                            scaledImage = zoomFit(image, &picturePortion, &scale);
                        }
                        name = drawFileName(argv[currentImageNumber], font, runSlideShow);
                    }
                    break;
                case SDLK_s: // START / STOP SLIDESHOW
                    runSlideShow = 1 - runSlideShow;
                    name = drawFileName(argv[currentImageNumber], font, runSlideShow);
                    if (runSlideShow) {
                        slideShowTimer = SDL_AddTimer(SLIDESHOWTIMEOUT, timerCallback, NULL);
                    } else {
                        SDL_RemoveTimer(slideShowTimer);
                    }
                    break;
                case SDLK_d: // SHOW / HIDE FILENAME
                    showFileName = 1 - showFileName;
                    break;
                case SDLK_ESCAPE: // QUIT
                case SDLK_q:
                    isRunning = FALSE;
                    break;
                default:
                    break;
             } // end of switch (event.key.keysym.sym)
        } // end of if(SDL_WaitEvent())
        drawImage(scaledImage, &picturePortion, screen, showFileName ? name : 0);
    } while(isRunning); // end of do

    SDL_FreeSurface(image);
    SDL_FreeSurface(scaledImage);
    SDL_FreeSurface(screen);

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_Quit();

    return 0;
}

