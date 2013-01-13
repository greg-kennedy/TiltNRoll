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

/* editor.cpp */
#include "editor.h"

extern char ERRMSG[80];

GLuint tex_editor[3];
GLuint tileset[32];
extern GLuint last_used_tex, cursor, gamestate;

extern Mix_Music *music;

extern int mx, my;
extern int sfxon;
extern GLuint tex_quality;
extern int SCREEN_X, SCREEN_Y;
extern TTF_Font *systemfont;

float editzoom,editx,edity;
unsigned int maxtextures, toptex, seltex, editheight, infopop;

s_levelset editing;
s_level *edlvl;

int nextpoweroftwo(int x)
{
	double logbase2 = log(x) / log(2);
	return (int)(pow(2,ceil(logbase2))+ .5);
}

void load_levelset(char *name)
{
     FILE *fp;
     char buffer[80];
     int i;
     
     s_level *cur;
     
     sprintf(buffer,"levels/%s",name);
     fp = fopen(buffer,"r");
     if (fp == NULL) return;

     cur = editing.top;
     while (cur != NULL) { s_level *tmp = cur->next; free(cur); cur = tmp; }
     
     fread(editing.name,1,50,fp);
     fread(editing.author,1,50,fp);
     fread(editing.version,1,10,fp);
     fread(&editing.tileset,1,1,fp);
     i=1;
     while (!feof(fp))
     {
           s_level *tmp = (s_level *)malloc(sizeof(s_level));
//           fread(tmp->name,1,30,fp);
           fread(&tmp->timelimit,1,1,fp);
           if (feof(fp)) free(tmp); else {
           fread(tmp->tile,1,4096,fp);
           tmp->num = i;
           if (i == 1)
           {
                 editing.top = tmp;
                 tmp->prev = NULL;
           } else {
                 tmp->prev = cur;
                cur->next = tmp;
           }
           i++;
           tmp->next = NULL;
           cur = tmp;
           }
     }
     fclose(fp);
     
     edlvl = editing.top;
     update_timelimit();
}

void save_levelset (char *name)
{
     FILE *fp;
     char buffer[80];
     
     s_level *cur;

     sprintf(buffer,"levels/%s",name);
     fp = fopen(buffer,"w");
     if (fp == NULL) return;
     fwrite(editing.name,1,50,fp);
     fwrite(editing.author,1,50,fp);
     fwrite(editing.version,1,10,fp);
     fwrite(&editing.tileset,1,1,fp);
     
     cur = editing.top;
     while (cur != NULL)
     {
           fwrite(&cur->timelimit,1,1,fp);
//           fwrite(cur->name,1,30,fp);
           fwrite(cur->tile,1,4096,fp);
           cur = cur->next;
     }
     fclose(fp);
}

void load_level()
{
     bool tinydone = false;
     GLuint tinytex[2];
     char setname[80] = {0};
    SDL_Event event;
    
     glGenTextures(2, tinytex);

     strcpy(setname, "testing.lvl");
     SDL_Surface *ts, *intermediary;
	SDL_Color color={255,255,0};


     ts = TTF_RenderText_Solid(systemfont,"Enter levelset name for *LOAD*:",color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     USETEX(tinytex[0]);
//     glBindTexture(GL_TEXTURE_2D, tinytex[0]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);

     ts = TTF_RenderText_Solid(systemfont,setname,color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     USETEX(tinytex[1]);
//     glBindTexture(GL_TEXTURE_2D, tinytex[1]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);
	
     while (!tinydone)
     {
           glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glColor3f(1,1,1);
    USETEX (tinytex[0]);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(200, 300);
      glTexCoord2f(1, 1);
      glVertex2i(700,300);
      glTexCoord2f(1, 0);
      glVertex2i(700,400);
      glTexCoord2f(0, 0);
      glVertex2i(200,400);
    glEnd();
    USETEX (tinytex[1]);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(200, 200);
      glTexCoord2f(1, 1);
      glVertex2i(700,200);
      glTexCoord2f(1, 0);
      glVertex2i(700,300);
      glTexCoord2f(0, 0);
      glVertex2i(200,300);
    glEnd();

    SDL_GL_SwapBuffers();
    SDL_Delay (1);

    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_KEYUP:
                  if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(setname) > 0)
                  {
                       if (strlen(setname) > 1)
						setname[strlen(setname)-1]='\0';
						else { setname[1]='\0';
						setname[0]=' ';}
     ts = TTF_RenderText_Solid(systemfont,setname,color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     USETEX( tinytex[1]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);
                    }
					else if (strlen(setname) < 80 && ((event.key.keysym.sym >= '0' && event.key.keysym.sym <= '9') || (event.key.keysym.sym >= 'a' && event.key.keysym.sym <= 'z') || event.key.keysym.sym == '.' || (event.key.keysym.sym >= 'A' && event.key.keysym.sym <= 'Z')))
					{
                         if (setname[0] == ' ') 
						setname[0]=(char)(event.key.keysym.sym);
                         else
						setname[strlen(setname)]=(char)(event.key.keysym.sym);
						
     ts = TTF_RenderText_Solid(systemfont,setname,color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     glBindTexture(GL_TEXTURE_2D, tinytex[1]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                           tinydone = true;
                    }
                 else if(event.key.keysym.sym == SDLK_ESCAPE) {
                       setname[0] = ' ';
                       tinydone = true;
                 }
                 break;
            case SDL_QUIT:
                 setname[0] = '\0';
                 tinydone = true;
                 break;
            default:
                    break;
        }
    }
           
     }
     glDeleteTextures( 2,tinytex );
     if (setname[0] != ' ')
     load_levelset(setname);
}

void save_level()
{
     bool tinydone = false;
     GLuint tinytex[2];
     char setname[80] = {0};
    SDL_Event event;
    
     glGenTextures(2, tinytex);

     strcpy(setname, "testing.lvl");
     SDL_Surface *ts, *intermediary;
	SDL_Color color={255,255,0};


     ts = TTF_RenderText_Solid(systemfont,"Enter levelset name for *SAVE*:",color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     USETEX(tinytex[0]);
//     glBindTexture(GL_TEXTURE_2D, tinytex[0]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);

     ts = TTF_RenderText_Solid(systemfont,setname,color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     USETEX(tinytex[1]);
//     glBindTexture(GL_TEXTURE_2D, tinytex[1]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);
	
     while (!tinydone)
     {
           glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glColor3f(1,1,1);
    USETEX (tinytex[0]);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(200, 300);
      glTexCoord2f(1, 1);
      glVertex2i(700,300);
      glTexCoord2f(1, 0);
      glVertex2i(700,400);
      glTexCoord2f(0, 0);
      glVertex2i(200,400);
    glEnd();
    USETEX (tinytex[1]);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(200, 200);
      glTexCoord2f(1, 1);
      glVertex2i(700,200);
      glTexCoord2f(1, 0);
      glVertex2i(700,300);
      glTexCoord2f(0, 0);
      glVertex2i(200,300);
    glEnd();

    SDL_GL_SwapBuffers();
    SDL_Delay (1);

    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_KEYUP:
                  if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(setname) > 0)
                  {
                       if (strlen(setname) > 1)
						setname[strlen(setname)-1]='\0';
						else { setname[1]='\0';
						setname[0]=' ';}
     ts = TTF_RenderText_Solid(systemfont,setname,color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     USETEX( tinytex[1]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);
                    }
					else if (strlen(setname) < 80 && ((event.key.keysym.sym >= '0' && event.key.keysym.sym <= '9') || (event.key.keysym.sym >= 'a' && event.key.keysym.sym <= 'z') || event.key.keysym.sym == '.' || (event.key.keysym.sym >= 'A' && event.key.keysym.sym <= 'Z')))
					{
                         if (setname[0] == ' ') 
						setname[0]=(char)(event.key.keysym.sym);
                         else
						setname[strlen(setname)]=(char)(event.key.keysym.sym);
						
     ts = TTF_RenderText_Solid(systemfont,setname,color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     glBindTexture(GL_TEXTURE_2D, tinytex[1]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                           tinydone = true;
                    }
                 else if(event.key.keysym.sym == SDLK_ESCAPE) {
                       setname[0] = ' ';
                       tinydone = true;
                 }
                 break;
            case SDL_QUIT:
                 setname[0] = '\0';
                 tinydone = true;
                 break;
            default:
                    break;
        }
    }
           
     }
     glDeleteTextures( 2,tinytex );
     if (setname[0] != ' ')
     save_levelset(setname);
}

void info_level(){ infopop = 1; }

void update_editheight ()
{
     SDL_Surface *ts, *intermediary;
	SDL_Color color={255,255,0};
	char hbuf[15];
	
	sprintf(hbuf,"Height: %d",editheight);

     ts = TTF_RenderText_Solid(systemfont,hbuf,color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     USETEX ( tex_editor[1]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);

}

void update_timelimit ()
{
     SDL_Surface *ts, *intermediary;
	SDL_Color color={255,255,0};
	char hbuf[25];
	
	sprintf(hbuf,"Time: %d",edlvl->timelimit);

     ts = TTF_RenderText_Solid(systemfont,hbuf,color);
     intermediary = SDL_CreateRGBSurface(0, nextpoweroftwo(ts->w), nextpoweroftwo(ts->h),
                  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

     SDL_BlitSurface(ts, NULL, intermediary, NULL);
     USETEX ( tex_editor[2]);
     glTexImage2D(GL_TEXTURE_2D, 0, 4, intermediary->w, intermediary->h, 0, GL_BGRA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	
	SDL_FreeSurface(ts);
	SDL_FreeSurface(intermediary);

}

void new_level(){
     s_level *cur;
     int i,j;
     
     cur = editing.top;
     while (cur != NULL) { s_level *tmp = cur->next; free(cur); cur = tmp; }
     
     editing.tileset = 0;
     strcpy(editing.name,  "New Levelset");
     strcpy(editing.author,"Unknown Author");
     strcpy(editing.version,"1.0");

     edlvl = (s_level *)malloc(sizeof(s_level));
     editing.top = edlvl;
     strcpy(edlvl->name, "New Level");
     edlvl->timelimit = 30;
     edlvl->next=NULL;
     edlvl->prev=NULL;
     
     for (i=0;i<64;i++)
     for (j=0;j<64;j++)
     edlvl->tile[i][j]=0;
     
     editzoom = 1;
     editx = 0; edity = 0;
     toptex = 0; seltex = 0;
     infopop = 0;
     editheight = 1;
     update_editheight();

}

void load_tileset(int number)
{
     SDL_Surface *ts;
     int i;
     char tspath[80];
     glGenTextures( 32, tileset );
     
     maxtextures = 0;

     for (i=0; i<32; i++)
     {
         sprintf(tspath,"img/tiles/%d/%d.png",number,i);
         ts = IMG_Load(tspath);
         if (ts == NULL) { i = 32; } else {
     USETEX ( tileset[i] );
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ts->w, ts->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ts->pixels);
	 SDL_FreeSurface(ts);
	 maxtextures++;
         }
     }
}

void init_editor()
{
     SDL_Surface *ts;
     char buffer[80];
     int i,j;

     glGenTextures( 3, tex_editor );

     for (i = 0; i < 3; i++)
     {
         USETEX ( tex_editor[i] );
         if (i < 1) {
         sprintf(buffer,"img/ui/editor/%d.png",i);
         ts = IMG_Load(buffer);
         if (ts == NULL) { sprintf(ERRMSG,"init_editor: Could not load %s",buffer); gamestate = -1; return; }
	     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ts->w, ts->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ts->pixels);
	     SDL_FreeSurface(ts);
         }
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
      }
      
      load_tileset(0);

     last_used_tex = 0;

	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
    glOrtho(0,800,0,600,0,100);
	glMatrixMode( GL_MODELVIEW ) ;
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

     if (music == NULL && sfxon) { music = Mix_LoadMUS("snd/editor.mod");
     Mix_PlayMusic(music,-1);
     }
     
     editing.tileset = 0;
//     editing.name = (char *)malloc(13);
     strcpy(editing.name,  "New Levelset");
//     editing.author = (char *)malloc(15);
     strcpy(editing.author,"Unknown Author");
//     editing.version = (char *)malloc(4);
     strcpy(editing.version,"1.0");

     edlvl = (s_level *)malloc(sizeof(s_level));
     editing.top = edlvl;
//     edlvl->name = (char *)malloc(10);
     strcpy(edlvl->name, "New Level");
     edlvl->timelimit = 30;
     edlvl->next=NULL;
     edlvl->prev=NULL;
     
     for (i=0;i<64;i++)
     for (j=0;j<64;j++)
     edlvl->tile[i][j]=0;
     
     editzoom = 1;
     editx = 0; edity = 0;
     toptex = 0; seltex = 0;
     infopop = 0;
     editheight = 1;
     update_timelimit();
     update_editheight();
}

void destroy_editor()
{
     glDeleteTextures( 3,tex_editor );
     glDeleteTextures(32, tileset);
     Mix_HaltMusic(); Mix_FreeMusic(music); music = NULL;
     
     s_level *tmplvl = editing.top;
     while (tmplvl != NULL)
     {
           editing.top = tmplvl->next;
//           free(tmplvl->name);
           free(tmplvl);
           tmplvl = editing.top;
     }
/*      free(editing.name);
      free(editing.author);
      free(editing.version);*/
}

void draw_editor()
{
     int i, j;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    if (!infopop) {

    USETEX(0);
    glPushMatrix();
    glTranslatef(160+editx,edity,0);
    glScalef(editzoom,editzoom,0);
//    glTranslatef(editx,edity,0);
    for (i=0; i<64; i++)
    for (j=0; j<64; j++)
    {
        if (edlvl->tile[i][j] != 0)
        {
           USETEX( tileset[(edlvl->tile[i][j] & 0x1F)] );
           glColor3f(1-(float)(edlvl->tile[i][j] >> 5)/8,(float)(edlvl->tile[i][j] >> 5)/8,0);
        } else {
           USETEX(0);
           glColor3f(0,0,1);
        }
//        USETEX(map[i][j]);
//        glColor3f(0,1,0);
//        if (i == 10) glColor3f(1,0,0);
//       if (j == 10) glColor3f(0,0,1);
        glBegin(GL_QUADS);
           glTexCoord2f(0, .5);
           glVertex2i(10*i, 9*j); // 24+9*j
           glTexCoord2f(1, .5);
           glVertex2i(10*(i+1),9*j);
           glTexCoord2f(1, 0);
           glVertex2i(10*(i+1),9*(j+1));
           glTexCoord2f(0, 0);
           glVertex2i(10*i,9*(j+1));
        glEnd();
    }
    glPopMatrix();
    
    for (i=0; i<8; i++)
    {
        if (i + toptex < maxtextures) {
           USETEX(tileset[i + toptex]);
        
        glColor3f(1,1,1);

        glBegin(GL_QUADS);
           glTexCoord2f(0, .5);
           glVertex2i(0, 472-(64*(i+1))); // 24+9*j
        if (i + toptex == seltex) glColor3f(1,0,0);
           glTexCoord2f(1, .5);
           glVertex2i(64,472-(64*(i+1)));
        if (i + toptex == seltex) glColor3f(0,1,0);
           glTexCoord2f(1, 0);
           glVertex2i(64,472-(64*i));
        if (i + toptex == seltex) glColor3f(0,0,1);
           glTexCoord2f(0, 0);
           glVertex2i(0,472-(64*i));
        glEnd();
        }
    }
//    glPopMatrix();
    
    glColor3f(1,1,1);
    USETEX (tex_editor[0]);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(0, 472);
      glTexCoord2f(1, 1);
      glVertex2i(64,472);
      glTexCoord2f(1, 0);
      glVertex2i(64,600);
      glTexCoord2f(0, 0);
      glVertex2i(0,600);
    glEnd();

    USETEX (tex_editor[1]);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(64, 568);
      glTexCoord2f(1, 1);
      glVertex2i(128,568);
      glTexCoord2f(1, 0);
      glVertex2i(128,600);
      glTexCoord2f(0, 0);
      glVertex2i(64,600);
    glEnd();

    USETEX (tex_editor[2]);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex2i(64, 536);
      glTexCoord2f(1, 1);
      glVertex2i(128,536);
      glTexCoord2f(1, 0);
      glVertex2i(128,568);
      glTexCoord2f(0, 0);
      glVertex2i(64,568);
    glEnd();

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
} else {
       
}

    SDL_GL_SwapBuffers();
    SDL_Delay (1);
}

bool handle_editor()
{
     int i,j;
    bool retval = false;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEBUTTONUP:
                 mx = event.button.x * 800 / SCREEN_X;
                 my = event.button.y * 600 / SCREEN_Y;
                 if (event.button.button == 4)
                   if (mx < 65) { if (toptex > 0) {toptex--;}} else editzoom += .5;
                 if (event.button.button == 5) {
                   if (mx < 65) { if (toptex + 7 < maxtextures) {toptex++;}} else {editzoom -= .5; if (editzoom < 1) {editzoom = 1; editx = 0; edity = 0;}}}
                 if (event.button.button == 1) {
                   if (mx < 65) {
                   if (my > 128) {seltex = toptex + ((my-128) / 64); if (seltex > maxtextures) seltex = maxtextures;}
                   else if (mx < 33)
                   {
                       if (my < 33) load_level();
                       else if (my < 65) new_level();
                       else if (my < 93) { if (edlvl->prev != NULL) edlvl = edlvl->prev;  update_timelimit();}
                       else { s_level *tmp; tmp = (s_level *)malloc(sizeof(s_level)); for (i=0; i<64; i++) for (j=0; j<64; j++) tmp->tile[i][j]=0; tmp->timelimit = 30; tmp->next = edlvl->next; if (tmp->next != NULL) tmp->next->prev = tmp; tmp->prev = edlvl; edlvl->next = tmp; }
                   } else {
                       if (my < 33) save_level();
                       else if (my < 65) info_level();
                       else if (my < 93) { if (edlvl->next != NULL) edlvl = edlvl->next; update_timelimit();}
                       else {
                         if (edlvl->next == NULL && edlvl->prev == NULL) {
                            for (i=0; i<64; i++) for (j=0; j<64; j++) edlvl->tile[i][j]=0; edlvl->timelimit = 30; 
                         } else {
                         s_level *tmp;
                         if (edlvl->prev == NULL)
                         {
                             editing.top = edlvl->next;
                         } else {
                             edlvl->prev->next = edlvl->next;
                         }
                         if (edlvl->next != NULL)
                           edlvl->next->prev = edlvl->prev;
                         if (edlvl->prev != NULL) tmp = edlvl->prev; else tmp = edlvl->next;
                         free(edlvl);
                         edlvl = tmp;
                         }
                         update_timelimit();
                       }
                   }
                 } else {
                      int q = (int)((mx-(160+editx))/(editzoom*10));
                      int r = (int)((600-my-edity)/(editzoom*9));
                      if (q >= 0 && q < 64 && r >= 0 && r < 64)
  
                      edlvl->tile[q][r] = (editheight << 5) | (seltex & 0x1F);                    
//                      edlvl->tile[q][r] = (edlvl->tile[q][r] & 0xE0) | (seltex & 0x1F);
//                      if ((edlvl->tile[q][r] >> 5) == 0) edlvl->tile[q][r] = edlvl->tile[q][r] | 0x20;

                 }
                 }
                 if (event.button.button == 3 && mx > 64) { 
              int q = (int)((mx-(160+editx))/(editzoom*10));
                      int r = (int)((600-my-edity)/(editzoom*9));
                      if (q >= 0 && q < 64 && r >= 0 && r < 64)
                      
                      edlvl->tile[q][r] = 0;
                      }
               break;
            case SDL_MOUSEMOTION:
                 mx = event.motion.x * 800 / SCREEN_X;
                 my = event.motion.y * 600 / SCREEN_Y;
                 break;
            case SDL_KEYUP:
                 if (event.key.keysym.sym == SDLK_LEFT)
                    editx+=32;
                 else if (event.key.keysym.sym == SDLK_RIGHT)
                    editx-=32;
                 else if (event.key.keysym.sym == SDLK_DOWN)
                    edity+=32;
                 else if (event.key.keysym.sym == SDLK_UP)
                    edity-=32;
                 else if (event.key.keysym.sym == SDLK_F3)
                   if (mx < 65) { if (toptex > 0) {toptex--;}} else editzoom += .5;
                 else if (event.key.keysym.sym == SDLK_F2) {
                   if (mx < 65) { if (toptex + 7 < maxtextures) {toptex++;}} else {editzoom -= .5; if (editzoom < 1) {editzoom = 1; editx = 0; edity = 0;}}}
                 else if (event.key.keysym.sym == SDLK_PAGEUP)
                 {
                    editheight++;
                    if (editheight > 7) editheight = 1;
                    update_editheight();
                  }
                 else if (event.key.keysym.sym == SDLK_PAGEDOWN)
                 {
                    editheight--;
                    if (editheight < 1) editheight = 7;
                    update_editheight();
                  }
                 else if (event.key.keysym.sym == SDLK_HOME)
                 {
                    edlvl->timelimit += 5;
                    if (edlvl->timelimit > 250) edlvl->timelimit = 250;
                    update_timelimit();
                  }
                 else if (event.key.keysym.sym == SDLK_END)
                 {
                    edlvl->timelimit -= 5;
                    if (edlvl->timelimit < 5) edlvl->timelimit = 5;
                    update_timelimit();
                  }
                 else if(event.key.keysym.sym == SDLK_ESCAPE)
                       gamestate = 0;
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
