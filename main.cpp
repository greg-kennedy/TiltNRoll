/*
  Tilt-n-Roll, copyright 2007 Greg Kennedy.  All rights reserved.
  
    Tilt-n-Roll is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Tilt-n-Roll is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Tilt-n-Roll; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA  
*/

#include "common.h"
#include "title.h"
#include "editor.h"
#include "game.h"

int gamestate;
char ERRMSG[80];
SDL_Surface *screen;

GLuint cursor;
GLuint last_used_tex;

int mx, my;
TTF_Font *systemfont;

Mix_Music *music = NULL;

unsigned long ticks;

int sfxon, res, flipxy, fullscreen;
GLuint tex_quality;
int SCREEN_X, SCREEN_Y;

#ifdef THREADED
SDL_mutex *lvlmutex, *objmutex, *partmutex;
#endif

char setname[80];

int sms_type;

SDL_Surface *loadImage(const char * filename)
{
	SDL_PixelFormat fmt =  {
		.palette = NULL,
		.BitsPerPixel = 32,
		.BytesPerPixel = 4,
		.Rloss = 0, .Gloss = 0, .Bloss = 0, .Aloss = 0,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		.Rshift = 24, .Gshift = 16, .Bshift = 8, .Ashift = 0,
		.Rmask = 0xFF000000, .Gmask = 0x00FF0000, .Bmask = 0x0000FF00, .Amask = 0x000000FF,
#else
		.Rshift = 0, .Gshift = 8, .Bshift = 16, .Ashift = 24,
		.Rmask = 0x000000FF, .Gmask = 0x0000FF00, .Bmask = 0x00FF0000, .Amask = 0xFF000000,
#endif
		.colorkey = 0,
		.alpha = 255
	};

	SDL_Surface * temp = IMG_Load(filename);
	if (temp == NULL) { sprintf(ERRMSG,"Failed to load %s", filename); return NULL; }
	SDL_Surface * ret = SDL_ConvertSurface(temp, &fmt, 0);
	if (ret == NULL) { sprintf(ERRMSG,"Failed to convert %s", filename); return ret; }
	SDL_FreeSurface(temp);
	return ret;
}

int main(int argc, char *argv[])
{
	int GLTexSize;
	bool done;
	SDL_Surface *ts;
	FILE *fp;

    srand(time(NULL));
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1){
		fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

    sfxon = 1;
    res = 2;
    tex_quality = GL_LINEAR;
    fullscreen = 1;
    flipxy = 0;
    
    fp = fopen("tnr.ini", "r");
    if (fp != NULL)
    {
           fscanf(fp,"%d\n%d\n%u\n%d\n%d\n",&sfxon,&res,&tex_quality,&fullscreen,&flipxy);
           fclose(fp);
    }

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 6 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );

	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 16);
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	
	if (res == 1) { SCREEN_X = 640; SCREEN_Y = 480; }
	else if (res == 2) {SCREEN_X = 800; SCREEN_Y = 600; }
	else {SCREEN_X = 1024; SCREEN_Y = 768;}
	fullscreen=0;
	screen = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, 0, SDL_OPENGL | (fullscreen==1?SDL_FULLSCREEN:0));
//	screen = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, 0, SDL_OPENGL | SDL_FULLSCREEN );
	if(screen == NULL){
		fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_WM_SetCaption ("Tilt-n-Roll", NULL);
	glViewport(0,0,SCREEN_X,SCREEN_Y);
//	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1);
	glEnable(GL_CULL_FACE);

	glEnable( GL_DEPTH_TEST ) ;
	glDepthFunc( GL_LESS );

//	glMatrixMode( GL_PROJECTION ) ;
//	glLoadIdentity() ;
//	glOrtho( -400, 400, -300, 300, -750, 750 ) ;
//	glOrtho( -SCREEN_X/2, SCREEN_X/2, -SCREEN_Y/2, SCREEN_Y/2, -750, 750 ) ;
	//    glOrtho( -SCREEN_X*2, SCREEN_X*2, -SCREEN_Y*2, SCREEN_Y*2, -1500, 1500 ) ;
//	glMatrixMode( GL_MODELVIEW ) ;
//	glEnable(GL_TEXTURE_2D);

	if(Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048)) {//4096)) {
		fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
		sfxon=0;
		//exit(1);
	}

	if (TTF_Init()==-1) { printf("TTF_Init: %s\n", TTF_GetError()); exit(2); }
	        systemfont=TTF_OpenFont("Vera.ttf", 10); if(!systemfont) { printf("TTF_OpenFont: %s\n", TTF_GetError()); exit(2);}

	printf("OpenGL information:\nVendor: %s\nRenderer: %s\nVersion: %s\nExtensions: %s\n",
			glGetString( GL_VENDOR ),
			glGetString( GL_RENDERER ),
			glGetString( GL_VERSION ),
			glGetString( GL_EXTENSIONS ) );
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, & GLTexSize ) ;
	printf("Max texture size: %d\n",GLTexSize);
	
	last_used_tex = 0;
	
	SDL_ShowCursor( SDL_DISABLE );
	glGenTextures(1, &cursor);
	ts = loadImage("img/cursor.png");
	if (ts == NULL) { fprintf(stderr, ERRMSG); SDL_ShowCursor(SDL_ENABLE); exit(3); } else {
	USETEX ( cursor );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ts->w, ts->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ts->pixels);
	SDL_FreeSurface(ts);
    }
    
    #ifdef UNIMOTION
    sms_type = detect_sms();
    printf("Detected SMS type %d (",sms_type);
    if (sms_type == 1) printf("powerbook)\n");
    else if (sms_type == 2) printf("ibook)\n");
    else if (sms_type == 3) printf("highrespb)\n");
    else if (sms_type == 4) printf("macbookpro)\n");
    else printf("unknown)\n");
    #endif

	gamestate = 0;
	done = false;
	while (!done)
	{
		if (gamestate == 0)
		{
			init_title();
			while (!done && gamestate == 0)
			{
				done = handle_title();
				draw_title();
			}
			destroy_title();
		} else if (gamestate == 1)
		{
			init_editor();
			while (!done && gamestate == 1)
			{
				draw_editor();
				done = handle_editor();
			}
			destroy_editor();
		} else if (gamestate == 10)
		{
			init_game();
#ifdef THREADED
			SDL_Thread *glthread;
			lvlmutex = SDL_CreateMutex();
			objmutex = SDL_CreateMutex();
			partmutex = SDL_CreateMutex();
			glthread = SDL_CreateThread(simulate_game, NULL);
if ( glthread == NULL ) {
        fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
        return 0;
    }

			while (!done && gamestate == 10)
			{
				draw_game();
				done = handle_game();
			}
			if (gamestate == 10) gamestate = 0;
            SDL_WaitThread(glthread, NULL);
            SDL_DestroyMutex ( lvlmutex );
            SDL_DestroyMutex ( objmutex );
            SDL_DestroyMutex ( partmutex );
#else
			while (!done && gamestate == 10)
			{
				draw_game();
				done = handle_game();
				simulate_game(NULL);
			}
#endif
			destroy_game();
		} else if (gamestate == -1)
		{
			fprintf(stderr,"%s\n",ERRMSG);
			done = true;
		} else {
			fprintf(stderr,"Game reached unknown gamestate %d.\n",gamestate);
			done = true;
		}
	}

    if (music != NULL)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
    }
    Mix_CloseAudio();

    fp = fopen("tnr.ini", "w");
    if (fp != NULL)
    {
           fprintf(fp,"%d\n%d\n%u\n%d\n%d\n",sfxon,res,tex_quality,fullscreen,flipxy);
           fclose(fp);
    }

	SDL_Quit();
	return(0);
}
