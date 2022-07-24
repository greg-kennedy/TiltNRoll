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

/* title.cpp */
#include "title.h"

extern int gamestate;
extern char ERRMSG[80], setname[80];

GLuint tex_title[8];
extern GLuint last_used_tex, cursor;

extern Mix_Music *music;
extern TTF_Font *systemfont;

static int title_choice=0, options_choice=0, optionspop=0;

extern int mx, my;

extern int sfxon, res, flipxy, fullscreen;
extern GLuint tex_quality;
extern int SCREEN_X, SCREEN_Y;

struct levelname
{
       char name[80];
       levelname *next;
       levelname *prev;
};
levelname *top, *curlvl;
SDL_Surface *statusSurf;

void title_drawLevelName(char *s)
{
	SDL_Color color={0,0,0};
    SDL_Surface *ts, *ts2;
    SDL_Rect rect;

	ts2 = SDL_ConvertSurface(statusSurf, statusSurf->format, statusSurf->flags);

	ts = TTF_RenderText_Solid(systemfont,s,color);
	rect.x = 0;
	rect.y = 8;
	SDL_BlitSurface(ts,NULL,ts2,&rect);
	SDL_FreeSurface(ts);

	USETEX ( tex_title[7] );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ts2->w, ts2->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ts2->pixels);
	SDL_FreeSurface(ts2);
}

void init_title()
{
     SDL_Surface *ts;
     char buffer[80];
     int i;
     
     DIR *dip;
     struct dirent *dit;
     levelname *tmplvlname, *tln2=NULL;
     top = NULL;
     
     if ((dip = opendir("levels")) == NULL) { sprintf(ERRMSG,"init_title: Could not open levels/ directory"); gamestate = -1; return; }
     while ((dit = readdir(dip)) != NULL)
     {
        if (strlen(dit->d_name) < 80 && strcmp(&(dit->d_name[strlen(dit->d_name)-4]),".lvl") == 0)
        {
           tmplvlname = (levelname*)malloc(sizeof(levelname));
           if (top != NULL) top->prev = tmplvlname; else tln2 = tmplvlname;
           tmplvlname->next = top;
           top = tmplvlname;
           strcpy(top->name,dit->d_name);
        }
        if (top != NULL) top->prev = tln2;
     }
     if (closedir(dip) == -1) { sprintf(ERRMSG,"init_title: Could not close levels/ directory"); gamestate = -1; return; }
     
     glGenTextures( 8, tex_title );

     for (i = 0; i < 8; i++)
     {
         sprintf(buffer,"img/ui/title/%d.png",i);
         ts = loadImage(buffer);
         if (ts == NULL) { gamestate = -1; return; }
         USETEX ( tex_title[i] );
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ts->w, ts->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ts->pixels);
	     if (i < 7) SDL_FreeSurface(ts); else statusSurf = ts;
      }
      
      if (top != NULL)
      {
              title_drawLevelName(top->name);
      }
      curlvl = top;

     USETEX(0);

	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
    glOrtho(0,800,0,600,0,100);
	glMatrixMode( GL_MODELVIEW ) ;
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

     if (music == NULL && sfxon) { music = Mix_LoadMUS("snd/title.mod");
     Mix_PlayMusic(music,-1);
     }
}

void destroy_title()
{
     if (top != NULL) strcpy(setname,curlvl->name); else setname[0]='\0';
     levelname *tmplvlname = top;
     while (tmplvlname != NULL)
     {
           top = top->next;
           free(tmplvlname);
           tmplvlname = top;
     }
     glDeleteTextures( 8,tex_title );
     
     Mix_HaltMusic(); Mix_FreeMusic(music); music = NULL;
}

void draw_title()
{
     int y;
//    glClear(GL_COLOR_BUFFER_BIT );
    
/*    fade += .01;
    if (fade > 1) fade = 1;

    glColor4f(1,1,1,fade);*/

     USETEX ( tex_title[0] );
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(0, 0);
      glTexCoord2f(1, 1);
      glVertex2i(800,0);
      glTexCoord2f(1, 0);
      glVertex2i(800,600);
      glTexCoord2f(0, 0);
      glVertex2i(0,600);
    glEnd();

     USETEX ( tex_title[7] );
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(283, 311);
      glTexCoord2f(1, 1);
      glVertex2i(526,311);
      glTexCoord2f(1, 0);
      glVertex2i(526,348);
      glTexCoord2f(0, 0);
      glVertex2i(283,348);
    glEnd();
    
    if (optionspop) {
       USETEX(tex_title[1]);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(272, 100);
      glTexCoord2f(1, 1);
      glVertex2i(528,100);
      glTexCoord2f(1, 0);
      glVertex2i(528,356);
      glTexCoord2f(0, 0);
      glVertex2i(272,356);
    glEnd();
}
    
    if (optionspop == 0) {
    switch (title_choice)
    {
           case 0:
               y = 224;
               break;
           case 1:
                y = 269;
                break;
           case 2:
                y = 352;
                break;
           case 3:
                y = 442;
                break;
           default:
                y = 530;
                break;           
    } } else {
        if (sfxon == 1) { USETEX(tex_title[2]); }
        else {USETEX(tex_title[3]);}
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex2i(476, 307);
            glTexCoord2f(1, 1);
            glVertex2i(508,307);
            glTexCoord2f(1, 0);
            glVertex2i(508,323);
            glTexCoord2f(0, 0);
            glVertex2i(476,323);
        glEnd();

        if (res == 1) { USETEX(tex_title[4]); }
        else if (res == 2) {USETEX(tex_title[5]);}
        else {USETEX(tex_title[6]);}
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex2i(460, 266);
            glTexCoord2f(1, 1);
            glVertex2i(524,266);
            glTexCoord2f(1, 0);
            glVertex2i(524,282);
            glTexCoord2f(0, 0);
            glVertex2i(460,282);
        glEnd();

        if (tex_quality == GL_LINEAR) { USETEX(tex_title[2]); }
        else {USETEX(tex_title[3]);}
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex2i(476, 244);
            glTexCoord2f(1, 1);
            glVertex2i(508,244);
            glTexCoord2f(1, 0);
            glVertex2i(508,260);
            glTexCoord2f(0, 0);
            glVertex2i(476,260);
        glEnd();

        if (fullscreen == 1) { USETEX(tex_title[2]); }
        else {USETEX(tex_title[3]);}
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex2i(476, 221);
            glTexCoord2f(1, 1);
            glVertex2i(508,221);
            glTexCoord2f(1, 0);
            glVertex2i(508,237);
            glTexCoord2f(0, 0);
            glVertex2i(476,237);
        glEnd();

        if (flipxy == 1) { USETEX(tex_title[2]); }
        else {USETEX(tex_title[3]);}
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex2i(476, 176);
            glTexCoord2f(1, 1);
            glVertex2i(508,176);
            glTexCoord2f(1, 0);
            glVertex2i(508,192);
            glTexCoord2f(0, 0);
            glVertex2i(476,192);
        glEnd();
        
        switch (options_choice)
    {
           case 0:
               y = 280;
               break;
           case 1:
                y = 325;
                break;
           case 2:
                y = 348;
                break;
           case 3:
                y = 370;
                break;
           case 4:
                y = 415;
                break;
           default:
                y = 477;
                break;           
    }}

    USETEX ( 0 );
    glPushMatrix();
      glColor3f(1,1,0);
    glTranslatef(220,600-y,0);
    glBegin(GL_TRIANGLES);
//      glColor3f(1,0,0);
      glVertex3f(16,0,-50);
//      glColor3f(0,1,0);
      glVertex3f(0,16,-50);
//      glColor3f(0,0,1);
      glVertex3f(0,-16,-50);
    glEnd();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(596,600-y,0);
    glBegin(GL_TRIANGLES);
//      glColor3f(1,0,0);
      glVertex3f(-16,0,-50);
//      glColor3f(0,0,1);
      glVertex3f(0,-16,-50);
//      glColor3f(0,1,0);
      glVertex3f(0,16,-50);
    glEnd();
    glPopMatrix();
      glColor3f(1,1,1);

      USETEX ( cursor );
    glBegin(GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex2i(mx, 600-my);
      glTexCoord2f(0, 1);
      glVertex2i(mx,568-my);
      glTexCoord2f(1, 1);
      glVertex2i(mx+32,568-my);
      glTexCoord2f(1, 0);
      glVertex2i(mx+32,600-my);
    glEnd();

    SDL_GL_SwapBuffers();
    SDL_Delay (1);
}

bool handle_title()
{
    bool retval = false;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEBUTTONUP:
                 mx = event.button.x * 800 / SCREEN_X;;
                 my = event.button.y * 600 / SCREEN_Y;
                 if (optionspop == 0) {
                 if (mx > 236 && mx < 580)
                 {
                        if (my >= 202 && my <= 248) gamestate = 10;
                        else if (my >= 250 && my <= 289 && top != NULL) { curlvl = curlvl->next; if (curlvl == NULL) curlvl = top; title_drawLevelName(curlvl->name);}
                        else if (my >= 333 && my <= 373) gamestate = 1;
                        else if (my >= 422 && my <= 472) optionspop = 1;
                        else if (my >= 508 && my <= 548) retval = true;
                 }
                 } else {
                        if (my >= 277 && my <= 290) { sfxon = !sfxon; }
                        else if (my >= 315 && my <= 330) { res--; if (res < 0) res = 2;}
                        else if (my >= 339 && my <= 353) { tex_quality = (tex_quality == GL_NEAREST?GL_LINEAR:GL_NEAREST);}
                        else if (my >= 361 && my <= 375) { fullscreen = !fullscreen; }
                        else if (my >= 408 && my <= 418) { flipxy = !flipxy;}
                        else if (my >= 468 && my <= 484) optionspop = 0;
                 }
               break;
            case SDL_MOUSEMOTION:
                 mx = event.motion.x * 800 / SCREEN_X;
                 my = event.motion.y * 600 / SCREEN_Y;
                 break;
            case SDL_KEYUP:
                 if (optionspop == 0) {
                 if (event.key.keysym.sym == SDLK_UP)
                 {
                    title_choice--;
                    if (title_choice < 0) title_choice = 4;
                 } else if (event.key.keysym.sym == SDLK_DOWN)
                 {
                    title_choice++;
                    if (title_choice > 4) title_choice = 0;
                 } else if (event.key.keysym.sym == SDLK_LEFT)
                 {
                        if (title_choice == 1 && top != NULL) { curlvl = curlvl->prev; title_drawLevelName(curlvl->name);}
                 } else if (event.key.keysym.sym == SDLK_RIGHT)
                 {
                        if (title_choice == 1 && top != NULL) { curlvl = curlvl->next; if (curlvl == NULL) curlvl = top; title_drawLevelName(curlvl->name);}
                 } else if (event.key.keysym.sym == SDLK_RETURN)
                 {
                    if (title_choice == 0) gamestate = 10;
                    else if (title_choice == 1 && top == NULL) {}
                    else if (title_choice == 1 && top != NULL) { curlvl = curlvl->next; if (curlvl == NULL) curlvl = top; title_drawLevelName(curlvl->name);}
                    else if (title_choice == 2) gamestate = 1;
                    else if (title_choice == 3) optionspop=1;
                    else retval=true;
                 } else if(event.key.keysym.sym == SDLK_ESCAPE)
                       retval = true;
                 } else { // optionspop == 1
                 if (event.key.keysym.sym == SDLK_UP)
                 {
                    options_choice--;
                    if (options_choice < 0) options_choice = 5;
                 } else if (event.key.keysym.sym == SDLK_DOWN)
                 {
                    options_choice++;
                    if (options_choice > 5) options_choice = 0;
                 } else if (event.key.keysym.sym == SDLK_LEFT)
                 {
                        if (options_choice == 0) { sfxon = !sfxon; }
                        else if (options_choice == 1) { res--; if (res < 0) res = 2;}
                        else if (options_choice == 2) { tex_quality = (tex_quality == GL_NEAREST?GL_LINEAR:GL_NEAREST);}
                        else if (options_choice == 3) { fullscreen = !fullscreen; }
                        else if (options_choice == 4) { flipxy = !flipxy;}
                 } else if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_RETURN)
                 {
                        if (options_choice == 0) { sfxon = !sfxon; }
                        else if (options_choice == 1) { res++; if (res > 2) res = 0;}
                        else if (options_choice == 2) { tex_quality = (tex_quality == GL_NEAREST?GL_LINEAR:GL_NEAREST);}
                        else if (options_choice == 3) { fullscreen = !fullscreen; }
                        else if (options_choice == 4) { flipxy = !flipxy;}
                        else if (event.key.keysym.sym == SDLK_RETURN) optionspop = 0;
                 } else if(event.key.keysym.sym == SDLK_ESCAPE)
                       optionspop = 0;
                 }
                 break;
            case SDL_QUIT:
                 retval = true;
                 break;
            default:
                    break;
        }
    }
    return retval;
}
