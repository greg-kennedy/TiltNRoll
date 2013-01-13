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

/* game.cpp */
#include "game.h"

// so first up, all those externs
extern int gamestate;
extern char ERRMSG[80], setname[80];
extern GLuint last_used_tex, cursor;
extern int mx, my;
extern int sfxon;
extern GLuint tex_quality;
extern int SCREEN_X, SCREEN_Y;
extern unsigned long ticks;
extern Mix_Music *music;
extern int sms_type;

#ifdef THREADED
extern SDL_mutex *lvlmutex, *objmutex, *partmutex;
#endif

// loads of sound effects.
Mix_Chunk *sfx[NUM_SFX];

TTF_Font *bigfont;

// opengl stuff
GLuint ballmodel, ballfx, spikeball;
GLuint bgImage;
GLuint maplist;
GLuint hud;
GLuint chipmodel, dalekmodel, dozermodel, fanshot, platform, keylist;
GLuint cratemodel, lockmodel;

GLuint tile[32];
GLuint boxtex[2];
GLuint partimgs[6];
SDL_Surface *hudsurf;

// ball position and all
float ballpx, ballpy, ballpz, ballvx, ballvy, ballvz, ballax, ballay, ballaz;
float ballrx, ballry;
float startx, starty;

// level info
s_levelset levelset;
s_level *level;
int lives;

unsigned long secsleft;

// other generics
float xrot=0, yrot = 0;
int powerup;
int playing_fallsound;
int leveldone;
bool needRedraw, needStatusUpdate;
int jumpok;
int numkeys;
unsigned int revok;
unsigned int spikeok;

struct object
{
       unsigned char type;
       float reg;
       float time;
       float x, y, z;
       float angle;
       int startx,starty;
       object *next, *prev;
};

object *topobject;

struct particle
{
       unsigned char type;
       float life;
       float x,y,z, ang;
       float vx, vy, vz, rot, az;
       particle *next, *prev;
};

particle *toppart;

void free_sounds()
{
	int i;
	Mix_HaltChannel(-1);
	for (i=0; i<NUM_SFX; i++)
		Mix_FreeChunk(sfx[i]);
}
void load_sounds()
{
	int i;
	char buffer[80];
	for (i=0; i<NUM_SFX; i++)
	{
		sprintf(buffer,"snd/fx/%d.wav",i);
		sfx[i] = Mix_LoadWAV(buffer);
		if (!sfx[i]) fprintf(stderr,"Error: couldn't load WAV file %s\n",buffer);
	}
}
void play_sound (int number, float x, float y, int volume = 128)
{
	int channel;
	float distance;
	Sint16 angle;

	angle = (Sint16)(90-(atan2(y-ballpy,x-ballpx) * 180 / 3.1415926));
	distance = sqrt((y-ballpy)*(y-ballpy) + (x-ballpx)*(x-ballpx)) * 20;
	if (sfxon && distance <= 100 && volume > 0)
	{
		channel = Mix_PlayChannel (-1, sfx[number], 0);
		if (distance > 20)
			Mix_SetPosition(channel, angle, (Uint8)distance);
//		if (volume != 128)
		    Mix_Volume(channel, volume);
	}
}

void createcloud ( int type, float x, float y, float z,
                   float vxr, float vyr, float vzr, float az,
                   float rc, int number, int shape, int life, float lifer)
{
     int i;
     float tang, tdst;
     particle *tpart;

#ifdef THREADED
    SDL_mutexP(partmutex);
#endif
    for (i=0; i<number; i++)
    {
        tpart = (particle *)malloc(sizeof(particle));
        if (tpart != NULL) {
            if (shape == 0) {
            tpart->x = x + .5 -((float)rand() / RAND_MAX);
            tpart->y = y + .5 -((float)rand() / RAND_MAX);
            tpart->z = z + .5 -((float)rand() / RAND_MAX);
            } else {
                   tang = ((float)rand() / RAND_MAX) * 6.283185306f;
                   tdst = ((float)rand() / RAND_MAX) / 2;
            tpart->x = x + (tdst * cos (tang));
            tpart->y = y + (tdst * sin (tang));
            tpart->z = z + .5 -((float)rand() / RAND_MAX);
            }
            tpart->vx = vxr * (((float)rand() / RAND_MAX) - .5);
            tpart->vy = vyr * (((float)rand() / RAND_MAX) - .5);
            tpart->vz = vzr * (((float)rand() / RAND_MAX) - .5);
            tpart->az = az;
            tpart->ang = ((float)rand() / RAND_MAX) * 360.0f;
            tpart->rot = rc * (((float)rand() / RAND_MAX) - .5) / 10;
            tpart->life = .5+life + (((float)rand()/ RAND_MAX) * lifer);
            
//            printf("created particle at %f,%f,%f life %f\n",tpart->x,tpart->y,tpart->z,tpart->life);
            tpart->type = type;
            tpart->next = toppart;
            if (toppart != NULL) toppart->prev = tpart;
            tpart->prev = NULL;
            toppart = tpart;
        }
    }
#ifdef THREADED
    SDL_mutexV(partmutex);
#endif
}

void drawdalek()
{
     float rad;
    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=360;i+=45)
    {
      rad = (float)i * 0.0174532925f;
      glColor3f(.25,.25,.25);
      glVertex3f(.4*cos(rad), .3*sin(rad), 0.3);
      glColor3f(.5,.5,0);
      glVertex3f(.5*cos(rad), .4*sin(rad), -.5);
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=360;i+=45)
    {
      rad = (float)i * 0.0174532925f;
      glColor3f(.5,.5,.5);
      glVertex3f(.1*cos(rad), .0667*sin(rad), .5);
      glColor3f(.25,.25,.25);
      glVertex3f(.4*cos(rad), .3*sin(rad), .3);
    }
    glEnd();

    glColor3f(1,0,0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, .5);
    for (int i=0;i<=360;i+=45)
    {
      rad = (float)i * 0.0174532925f;
      glVertex3f(.1*cos(rad), .0667*sin(rad),.5);
    }
    glEnd();
}

void drawdozer()
{
    glColor3f(.75,.75,0);
    glBegin(GL_QUADS);
      glTexCoord2f(1, 0);
      glVertex3f(-.4,.15,.5);
      glTexCoord2f(1, 1);
      glVertex3f(-.4,-.15,.5);
      glTexCoord2f(0, 1);
      glVertex3f(0,-.15,.5);
      glTexCoord2f(0, 0);
      glVertex3f(0,.15,.5);
   glEnd();

    glColor3f(.6,.6,0);
    glBegin(GL_QUADS);
      glTexCoord2f(1, 0);
      glVertex3f(-.5,.3,.1);
      glTexCoord2f(1, 1);
      glVertex3f(-.5,-.3,.1);
      glTexCoord2f(0, 1);
      glVertex3f(.2,-.3,.1);
      glTexCoord2f(0, 0);
      glVertex3f(.2,.3,.1);
   glEnd();

    glColor3f(.5,.5,0);
    glBegin(GL_QUAD_STRIP);
      glTexCoord2f(0, 0);
      glVertex3f(-.4,.15,.1);
      glTexCoord2f(1, 0);
      glVertex3f(-.4,.15,0.5);
      glTexCoord2f(1, 1);
      glVertex3f(0,.15,.1);
      glTexCoord2f(0, 1);
      glVertex3f(0,.15,.5);
      glTexCoord2f(1, 0);
      glVertex3f(0,-.15,.1);
      glTexCoord2f(0, 0);
      glVertex3f(0,-.15,.5);
      glTexCoord2f(1, 1);
      glVertex3f(-.4,-.15,.1);
      glTexCoord2f(0, 1);
      glVertex3f(-.4,-.15,.5);
      glTexCoord2f(1, 0);
      glVertex3f(-.4,.15,.1);
      glTexCoord2f(0, 0);
      glVertex3f(-.4,.15,.5);
    glEnd();

    glColor3f(.4,.4,0);
    glBegin(GL_QUAD_STRIP);
      glTexCoord2f(0, 0);
      glVertex3f(-.5,.3,-.4);
      glTexCoord2f(1, 0);
      glVertex3f(-.5,.3,0.1);
      glTexCoord2f(1, 1);
      glVertex3f(0.2,.3,-.4);
      glTexCoord2f(0, 1);
      glVertex3f(0.2,.3,.1);
      glTexCoord2f(1, 0);
      glVertex3f(0.2,-.3,-.4);
      glTexCoord2f(0, 0);
      glVertex3f(0.2,-.3,.1);
      glTexCoord2f(1, 1);
      glVertex3f(-.5,-.3,-.4);
      glTexCoord2f(0, 1);
      glVertex3f(-.5,-.3,.1);
      glTexCoord2f(1, 0);
      glVertex3f(-.5,.3,-.4);
      glTexCoord2f(0, 0);
      glVertex3f(-.5,.3,.1);
    glEnd();
    
    glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(.4,.4,.4);
    glVertex3f(.25,.4,.2);
    glColor3f(.65,.65,.65);
    glVertex3f(.5,.4,-.4);
    glColor3f(.4,.4,.4);
    glVertex3f(.25,.4,-.4);
    glColor3f(.65,.65,.65);
    glVertex3f(.5,-.4,-.4);
    glColor3f(.4,.4,.4);
    glVertex3f(.25,-.4,-.4);
    glColor3f(.65,.65,.65);
    glVertex3f(.25,-.4,.2);
    glColor3f(.4,.4,.4);
    glVertex3f(.25,.4,-.4);
    glColor3f(.65,.65,.65);
    glVertex3f(.25,.4,.2);
    glEnd();
    
    glBegin(GL_QUAD_STRIP);
    glColor3f(.4,.4,.4);
    glVertex3f(-.5,-.5,-.5);
    glVertex3f(-.5,-.3,-.5);
    glColor3f(.65,.65,.65);
    glVertex3f(.2,-.5,-.5);
    glVertex3f(.2,-.3,-.5);
    glColor3f(.4,.4,.4);
    glVertex3f(.2,-.5,-.3);
    glVertex3f(.2,-.3,-.3);
    glColor3f(.65,.65,.65);
    glVertex3f(-.5,-.5,-.3);
    glVertex3f(-.5,-.3,-.3);
    glColor3f(.4,.4,.4);
    glVertex3f(-.5,-.5,-.5);
    glVertex3f(-.5,-.3,-.5);
    glEnd();

    glBegin(GL_QUAD_STRIP);
    glColor3f(.4,.4,.4);
    glVertex3f(-.5,.5,-.5);
    glVertex3f(-.5,.3,-.5);
    glColor3f(.65,.65,.65);
    glVertex3f(.2,.5,-.5);
    glVertex3f(.2,.3,-.5);
    glColor3f(.4,.4,.4);
    glVertex3f(.2,.5,-.3);
    glVertex3f(.2,.3,-.3);
    glColor3f(.65,.65,.65);
    glVertex3f(-.5,.5,-.3);
    glVertex3f(-.5,.3,-.3);
    glColor3f(.4,.4,.4);
    glVertex3f(-.5,.5,-.5);
    glVertex3f(-.5,.3,-.5);
    glEnd();
   
    glEnable(GL_CULL_FACE);
    
    glColor3f(1,1,1);
}

void drawchip()
{
     float i;
    glBegin(GL_QUADS);
    glColor3f(.15,.15,.15);
      glTexCoord2f(1, 0);
      glVertex3f(-.5,.25,0);
      glTexCoord2f(1, 1);
      glVertex3f(-.5,-.25,0);
    glColor3f(.35,.35,.35);
      glTexCoord2f(0, 1);
      glVertex3f(.5,-.25,0);
      glTexCoord2f(0, 0);
      glVertex3f(.5,.25,0);
   glEnd();
    glColor3f(.1,.1,.1);
    glBegin(GL_QUAD_STRIP);
      glTexCoord2f(0, 0);
      glVertex3f(-.5,.25,-.25);
      glTexCoord2f(1, 0);
      glVertex3f(-.5,.25,0);
      glTexCoord2f(1, 1);
      glVertex3f(.5,.25,-.25);
      glTexCoord2f(0, 1);
      glVertex3f(.5,.25,0);
      glTexCoord2f(1, 0);
      glVertex3f(.5,-.25,-.25);
      glTexCoord2f(0, 0);
      glVertex3f(.5,-.25,0);
      glTexCoord2f(1, 1);
      glVertex3f(-.5,-.25,-.25);
      glTexCoord2f(0, 1);
      glVertex3f(-.5,-.25,0);
      glTexCoord2f(1, 0);
      glVertex3f(-.5,.25,-.25);
      glTexCoord2f(0, 0);
      glVertex3f(-.5,.25,0);
    glEnd();
    glColor3f(.5,.5,.5);
    glBegin(GL_TRIANGLES);
    for (i=-.5; i<.4; i+=.1)
    {
        glVertex3f(i+.05,-.25,-.5);
        glVertex3f(i+.1,-.25,-.25);
        glVertex3f(i,-.25,-.25);
        glVertex3f(i+.1,.25,-.5);
        glVertex3f(i,.25,-.25);
        glVertex3f(i+.05,.25,-.25);
    }
    glEnd();
    glColor3f(1,1,1);
}

void drawbox()
{
     glColor3f(1,1,1);
    glBegin(GL_QUADS);
      glTexCoord2f(1, 0);
      glVertex3f(-.4375,.4375,.4375);
      glTexCoord2f(1, 1);
      glVertex3f(-.4375,-.4375,.4375);
      glTexCoord2f(0, 1);
      glVertex3f(.4375,-.4375,.4375);
      glTexCoord2f(0, 0);
      glVertex3f(.4375,.4375,.4375);
   glEnd();
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1);
      glVertex3f(-.4375,.4375,-.4375);
      glTexCoord2f(1, 1);
      glVertex3f(.4375,.4375,-.4375);
      glTexCoord2f(1, 0);
      glVertex3f(.4375,-.4375,-.4375);
      glTexCoord2f(0, 0);
      glVertex3f(-.4375,-.4375,-.4375);
   glEnd();
    glBegin(GL_QUAD_STRIP);
      glTexCoord2f(0, 0);
      glVertex3f(-.4375,.4375,-.4375);
      glTexCoord2f(1, 0);
      glVertex3f(-.4375,.4375,.4375);
      glTexCoord2f(1, 1);
      glVertex3f(.4375,.4375,-.4375);
      glTexCoord2f(0, 1);
      glVertex3f(.4375,.4375,.4375);
      glTexCoord2f(1, 0);
      glVertex3f(.4375,-.4375,-.4375);
      glTexCoord2f(0, 0);
      glVertex3f(.4375,-.4375,.4375);
      glTexCoord2f(1, 1);
      glVertex3f(-.4375,-.4375,-.4375);
      glTexCoord2f(0, 1);
      glVertex3f(-.4375,-.4375,.4375);
      glTexCoord2f(1, 0);
      glVertex3f(-.4375,.4375,-.4375);
      glTexCoord2f(0, 0);
      glVertex3f(-.4375,.4375,.4375);
    glEnd();
    USETEX(0);
}

void drawfanshot()
{
     float rad;
//     USETEX(0);
     glDisable(GL_CULL_FACE);
     glEnable(GL_BLEND);
    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=360;i+=30)
    {
      rad = (float)i * 0.0174532925f;
    glColor4f(1,1,1,1);
      glVertex3f(.5,(.4*cos(rad)), (.4*sin(rad)));
    glColor4f(1,1,1,0);
      glVertex3f(-.5,(.4*cos(rad)), (.4*sin(rad)));
    }
    glEnd();
     glDisable(GL_BLEND);
     glEnable(GL_CULL_FACE);
}

void drawplatform()
{
    glBegin(GL_QUADS);
      glVertex3f(-.4375,.4375,.5);
      glVertex3f(-.4375,-.4375,.5);
      glVertex3f(.4375,-.4375,.5);
      glVertex3f(.4375,.4375,.5);
   glEnd();
   glColor3f(.25,.25,.25);
    glBegin(GL_QUAD_STRIP);
      glVertex3f(-.4375,.4375,-.5);
      glVertex3f(-.4375,.4375,.5);
      glVertex3f(.4375,.4375,-.5);
      glVertex3f(.4375,.4375,.5);
      glVertex3f(.4375,-.4375,-.5);
      glVertex3f(.4375,-.4375,.5);
      glVertex3f(-.4375,-.4375,-.5);
      glVertex3f(-.4375,-.4375,.5);
      glVertex3f(-.4375,.4375,-.5);
      glVertex3f(-.4375,.4375,.5);
    glEnd();
}

void drawkeylist()
{
     float rad;
     glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLE_FAN);
      glColor3f(1,.75,0);
        glVertex3f(-.3, 0, 0);
      glColor3f(1,1,0);
    for (int i=0;i<=360;i+=30)
    {
      rad = (float)i * 0.0174532925f;
      glVertex3f(.2*cos(rad)-.3, .2*sin(rad), 0);
    }
    glEnd();

    glBegin(GL_QUADS);
      glColor3f(1,.75,0);
      glVertex3f(-.2,.05,0);
      glColor3f(1,1,0);
      glVertex3f(-.2,-.05,0);
      glColor3f(1,.75,0);
      glVertex3f(.5,-.05,0);
      glColor3f(1,1,0);
      glVertex3f(.5,.05,0);
   glEnd();
    glBegin(GL_QUADS);
      glColor3f(1,.75,0);
      glVertex3f(.4,-.05,0);
      glColor3f(1,1,0);
      glVertex3f(.4,-.25,0);
      glColor3f(1,.75,0);
      glVertex3f(.5,-.25,0);
      glColor3f(1,1,0);
      glVertex3f(.5,-.05,0);
   glEnd();
    glBegin(GL_QUADS);
      glColor3f(1,1,0);
      glVertex3f(.3,-.05,0);
      glColor3f(1,.75,0);
      glVertex3f(.3,-.25,0);
      glColor3f(1,1,0);
      glVertex3f(.2,-.25,0);
      glColor3f(1,.75,0);
      glVertex3f(.2,-.05,0);
   glEnd();
     glEnable(GL_CULL_FACE);
}

void drawbumper (int x, int y, int z)
{
     float rad;
    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=360;i+=45)
    {
      rad = (float)i * 0.0174532925f;
      glColor3f(1,0,0);
      glVertex3f(.5+(float)x+(.35*cos(rad)), .5+(float)y+(.35*sin(rad)), (float)z+0.8);
      glColor3f(1,1,1);
      glVertex3f(.5+(float)x+(.35*cos(rad)), .5+(float)y+(.35*sin(rad)), (float)z);
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=360;i+=45)
    {
      rad = (float)i * 0.0174532925f;
      glColor3f(1,1,1);
      glVertex3f(.5+(float)x+(.1*cos(rad)), .5+(float)y+(.1*sin(rad)), (float)z+1.0);
      glColor3f(1,0,0);
      glVertex3f(.5+(float)x+(.35*cos(rad)), .5+(float)y+(.35*sin(rad)), (float)z+0.8);
    }
    glEnd();

    glDisable(GL_CULL_FACE);
    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=360;i+=45)
    {
      rad = (float)i * 0.0174532925f;
      glColor3f(0,0,1);
      glVertex3f(.5+(float)x+(.4*cos(rad)), .5+(float)y+(.4*sin(rad)), (float)z+0.7);
      glColor3f(.5,.5,1);
      glVertex3f(.5+(float)x+(.5*cos(rad)), .5+(float)y+(.5*sin(rad)), (float)z+0.5);
    }
    glEnd();
    glEnable(GL_CULL_FACE);

    glColor3f(1,1,1);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(.5+(float)x, .5+(float)y, (float)z+1.0);
    for (int i=0;i<=360;i+=45)
    {
      rad = (float)i * 0.0174532925f;
      glVertex3f(.5+(float)x+(.1*cos(rad)), .5+(float)y+(.1*sin(rad)),(float)z+1.0);
    }
    glEnd();
}

void drawfan(int x, int y, int z, int ang)
{
     float rad;
//     glPushMatrix();
    glBegin(GL_TRIANGLE_FAN);

    glColor3f(.9,.9,.9);
    glVertex3f((float)x+.5, (float)y+.5, (float)z+.5);
      glColor3f(0,0,0);
    for (int i=0;i<=360;i+=45)
    {
      rad = (float)i * 0.0174532925f;
      glVertex3f((float)x+.25*cos(rad)+.5, (float)y+.25*sin(rad)+.5,(float)z);
    }

    glEnd();

    glDisable(GL_CULL_FACE);
    glBegin(GL_QUAD_STRIP);
    if (ang) {
    for (int i=0;i<=360;i+=30)
    {
      rad = (float)i * 0.0174532925f;
    glColor3f(.25,.25,.25);
      glVertex3f((float)x+(.45*cos(rad))+.5,(float)y+.4, (float)z+(.45*sin(rad))+.5);
    glColor3f(.5,.5,.5);
      glVertex3f((float)x+(.45*cos(rad))+.5,(float)y, (float)z+(.45*sin(rad))+.5);
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=360;i+=30)
    {
      rad = (float)i * 0.0174532925f;
    glColor3f(.25,.25,.25);
      glVertex3f((float)x+(.45*cos(rad))+.5,(float)y+.6, (float)z+(.45*sin(rad))+.5);
    glColor3f(.5,.5,.5);
      glVertex3f((float)x+(.45*cos(rad))+.5,(float)y+1.0, (float)z+(.45*sin(rad))+.5);
    }
} else {
    for (int i=0;i<=360;i+=30)
    {
      rad = (float)i * 0.0174532925f;
    glColor3f(.25,.25,.25);
      glVertex3f((float)x+.4,(float)y+(.45*cos(rad))+.5, (float)z+(.45*sin(rad))+.5);
    glColor3f(.5,.5,.5);
      glVertex3f((float)x,(float)y+(.45*cos(rad))+.5, (float)z+(.45*sin(rad))+.5);
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=360;i+=30)
    {
      rad = (float)i * 0.0174532925f;
    glColor3f(.25,.25,.25);
      glVertex3f((float)x+0.6,(float)y+(.45*cos(rad))+.5, (float)z+(.45*sin(rad))+.5);
    glColor3f(.5,.5,.5);
      glVertex3f((float)x+1.0,(float)y+(.45*cos(rad))+.5, (float)z+(.45*sin(rad))+.5);
    }

       }
    glEnd();
    glEnable(GL_CULL_FACE);
//    glPopMatrix();
}



// stuff that is just for creating the ball.
#define X .525731112119133606 
#define Z .850650808352039932

static GLfloat vdata[12][3] = {    
    {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},    
    {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},    
    {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
};
static GLuint tindices[20][3] = { 
    {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
    {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
    {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
    {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };
void normalize(GLfloat *a) {
    GLfloat d=sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
    a[0]/=d; a[1]/=d; a[2]/=d;
}
void drawtri(GLfloat *a, GLfloat *b, GLfloat *c, int div, float r, int iscolored) {
    if (div<=0) {
        //glNormal3fv(a); 
        glVertex3f(a[0]*r, a[1]*r, a[2]*r);
        //glNormal3fv(c); 
        glVertex3f(c[0]*r, c[1]*r, c[2]*r);
        //glNormal3fv(b); 
        glVertex3f(b[0]*r, b[1]*r, b[2]*r);
    } else {
        GLfloat ab[3], ac[3], bc[3];
        for (int i=0;i<3;i++) {
            ab[i]=(a[i]+b[i])/2;
            ac[i]=(a[i]+c[i])/2;
            bc[i]=(b[i]+c[i])/2;
        }
        normalize(ab); normalize(ac); normalize(bc);
        float color = (float)rand() / RAND_MAX;
if (iscolored) glColor3f(color,color,color);
        drawtri(a, ab, ac, div-1, r, iscolored);
        drawtri(b, bc, ab, div-1, r, iscolored);
        drawtri(c, ac, bc, div-1, r, iscolored);
        drawtri(ab, bc, ac, div-1, r, iscolored);  //<--Comment this line and sphere looks really cool!
    }
}
void drawsphere(int ndiv, float radius=1.0, int iscolored=1) {
    glBegin(GL_TRIANGLES);
    for (int i=0;i<20;i++)
        drawtri(vdata[tindices[i][0]], vdata[tindices[i][1]], vdata[tindices[i][2]], ndiv, radius, iscolored);
    glEnd();
}

void drawspike()
{
     float rad;
    glBegin(GL_TRIANGLE_FAN);

    glColor3f(.9,.9,.9);
    glVertex3f(0, 0, 1.0);
      glColor3f(0,0,0);
    for (int i=0;i<=360;i+=45)
    {
      rad = (float)i * 0.0174532925f;
      glVertex3f(.25*cos(rad), .25*sin(rad), 0);
    }
    glEnd();
}

void drawspikeball()
{
      glPushMatrix();
        drawspike();
      glPopMatrix();
      glPushMatrix();
        glRotatef(90,1,0,0);
        drawspike();
      glPopMatrix();
      glPushMatrix();
        glRotatef(180,1,0,0);
        drawspike();
      glPopMatrix();
      glPushMatrix();
        glRotatef(270,1,0,0);
        drawspike();
      glPopMatrix();
      glPushMatrix();
        glRotatef(90,0,1,0);
        drawspike();
      glPopMatrix();
      glPushMatrix();
        glRotatef(270,0,1,0);
        drawspike();
      glPopMatrix();
}

void drawlight(int x, int y, int z, int color) {
     float rad;
    glBegin(GL_QUAD_STRIP);
    for (int i=0;i<=360;i+=30)
    {
      rad = (float)i * 0.0174532925f;
      glColor4f(1,1,1,.75);
      glVertex3f(.5+(float)x+(.45*cos(rad)), .5+(float)y+(.45*sin(rad)), (float)z);
      switch(color)
      {
        case 0:
          glColor4f(1,0,0,0);
          break;
        case 1:
          glColor4f(1,1,0,0);
          break;
        case 2:
        case 3:
        case 4:
          glColor4f(0,1,0,0);
          break;
        case 5:
          glColor4f(.5,.5,1,0);
          break;
        case 6:
          glColor4f(1,1,0,0);
          break;
        default:
          break;
      }
      glVertex3f(.5+(float)x+(.5*cos(rad)), .5+(float)y+(.5*sin(rad)), (float)z+4.0);
    }
    glEnd();
}

void game_load_levelset(char *name)
{
     FILE *fp;
     char buffer[80];
     int i;
     
     s_level *cur;
     
     cur = levelset.top;
     while (cur != NULL) { s_level *tmp = cur->next; free(cur); cur = tmp; }
     
     sprintf(buffer,"levels/%s",name);
     fp = fopen(buffer,"r");
     if (fp == NULL) return;
     fread(levelset.name,1,50,fp);
     fread(levelset.author,1,50,fp);
     fread(levelset.version,1,10,fp);
     fread(&levelset.tileset,1,1,fp);
     i=1;
     while (!feof(fp))
     {
           s_level *tmp = (s_level *)malloc(sizeof(s_level));
//           fread(tmp->name,1,30,fp);
             fread (&tmp->timelimit,1,1,fp);
           if (feof(fp)) free(tmp); else {
           fread(tmp->tile,1,4096,fp);
           tmp->num = i;
           if (i == 1)
           {
                 levelset.top = tmp;
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
     
     level = levelset.top;
}

void drawMap() // beginnings of map render - must stretch clamped textures sometime
{
     int i, j, h;
     float rad;

     glColor3f(1,1,1);
     for (i=0; i<64; i++)
     {
        for (j=0; j<64; j++)
        {
             if (((level->tile[i][j] >> 5) > 0) || (i == 0 || (level->tile[i][j] >> 5) > (level->tile[i-1][j] >> 5)) ||
               (j == 0 || (level->tile[i][j] >> 5) > (level->tile[i][j-1] >> 5)) || 
               (i == 63 || (level->tile[i][j] >> 5) > (level->tile[i+1][j] >> 5)) ||
               (j == 63 || (level->tile[i][j] >> 5) > (level->tile[i][j+1] >> 5)) )
               {  // check that at least one condition is met...
           USETEX( tile[(level->tile[i][j] & 0x1F)] );
           glBegin(GL_QUADS);
             if ((level->tile[i][j] >> 5) > 0) { // is this a hole?
                 glTexCoord2f(0, .5);
                 glVertex3i(i, j,(level->tile[i][j] >> 5));
                 glTexCoord2f(1, .5);
                 glVertex3i(i+1,j,(level->tile[i][j] >> 5));
                 glTexCoord2f(1, 0);
                 glVertex3i(i+1,j+1,(level->tile[i][j] >> 5));
                 glTexCoord2f(0, 0);
                 glVertex3i(i,j+1,(level->tile[i][j] >> 5));
             }
             
             if (i == 0 || (level->tile[i][j] >> 5) > (level->tile[i-1][j] >> 5)) // left side
             {
                   if (i == 0) h=0; else h=(level->tile[i-1][j] >> 5);
                glTexCoord2f(0, .5+(float)((level->tile[i][j] >> 5)-h)/2);
                glVertex3i(i,j+1,h);
                glTexCoord2f(1, .5+(float)((level->tile[i][j] >> 5)-h)/2);
                glVertex3i(i, j,h);
                glTexCoord2f(1, 0.5);
                glVertex3i(i, j,(level->tile[i][j] >> 5));
                glTexCoord2f(0, 0.5);
                glVertex3i(i,j+1,(level->tile[i][j] >> 5));
             }
             if (j == 0 || (level->tile[i][j] >> 5) > (level->tile[i][j-1] >> 5)) // bottom side
             {
                   if (j == 0) h=0; else h=(level->tile[i][j-1] >> 5);
                glTexCoord2f(0, .5+(float)((level->tile[i][j] >> 5)-h)/2);
                glVertex3i(i, j,h);
                glTexCoord2f(1, .5+(float)((level->tile[i][j] >> 5)-h)/2);
                glVertex3i(i+1,j,h);
                glTexCoord2f(1, 0.5);
                glVertex3i(i+1,j,(level->tile[i][j] >> 5));
                glTexCoord2f(0, 0.5);
                glVertex3i(i, j,(level->tile[i][j] >> 5));
              }

             if (i == 63 || (level->tile[i][j] >> 5) > (level->tile[i+1][j] >> 5)) // right side
             {
                   if (i == 63) h=0; else h=(level->tile[i+1][j] >> 5);
                glTexCoord2f(0, .5+(float)((level->tile[i][j] >> 5)-h)/2);
                glVertex3i(i+1,j,h);
                glTexCoord2f(1, .5+(float)((level->tile[i][j] >> 5)-h)/2);
                glVertex3i(i+1,j+1,h);
                glTexCoord2f(1, 0.5);
                glVertex3i(i+1,j+1,(level->tile[i][j] >> 5));
                glTexCoord2f(0, 0.5);
                glVertex3i(i+1,j,(level->tile[i][j] >> 5));
             }

             if (j == 63 || (level->tile[i][j] >> 5) > (level->tile[i][j+1] >> 5)) // top side
             {
                if (j == 63) h=0; else h=(level->tile[i][j+1] >> 5);
                glTexCoord2f(0, .5+(float)((level->tile[i][j] >> 5)-h)/2);
                glVertex3i(i+1,j+1,h);
                glTexCoord2f(1, .5+(float)((level->tile[i][j] >> 5)-h)/2);
                glVertex3i(i,j+1,h);
                glTexCoord2f(1, 0.5);
                glVertex3i(i,j+1,(level->tile[i][j] >> 5));
                glTexCoord2f(0, 0.5);
                glVertex3i(i+1,j+1,(level->tile[i][j] >> 5));
             }

           glEnd();
           }
// statics: fan, bumper, swinging gate
           if ((level->tile[i][j] & 0x1F) == 28 || (level->tile[i][j] & 0x1F) == 29)
           {
              USETEX(0);
              drawfan(i,j,level->tile[i][j] >> 5, (level->tile[i][j] & 0x1F) == 29);
              glColor3f(1,1,1);
           }
           if ((level->tile[i][j] & 0x1F) == 30)
           {
              USETEX(0);
              drawbumper(i,j,level->tile[i][j] >> 5);
              glColor3f(1,1,1);
           }
         }
     }

     // now, the static effects - map lights and such
     USETEX(0);
     glDisable(GL_CULL_FACE);
	 glDisable(GL_ALPHA_TEST);
//	 glDisable(GL_TEXTURE_2D);
     glDepthMask(GL_FALSE);
	 glEnable(GL_BLEND);
     for (i=0; i<64; i++)
     {
        for (j=0; j<64; j++)
        {
            //printf("%d ", level->tile[i][j] & 0x1F);
           if ((level->tile[i][j] & 0x1F) > 20 && (level->tile[i][j] & 0x1F) < 28) drawlight(i,j,level->tile[i][j] >> 5, (level->tile[i][j] & 0x1F)-21);
           else if ((level->tile[i][j] & 0x1F) == 18)
           {
                
//     USETEX(0);
//printf("drawing eleshot\n");
    glBegin(GL_QUAD_STRIP);
    for (int h=0;h<=360;h+=30)
    {
      rad = (float)h * 0.0174532925f;
    glColor4f(1,1,1,1);
      glVertex3f((.4*cos(rad))+(float)i+.5, (.4*sin(rad))+(float)j+.5,(float)(level->tile[i][j] >> 5));
    glColor4f(1,1,1,0);
      glVertex3f((.4*cos(rad))+(float)i+.5, (.4*sin(rad))+(float)j+.5,(float)(level->tile[i][j] >> 5)+1.0f);
    }
    glEnd();
    glColor4f(1,1,1,1);
}

//           if ((level->tile[i][j] & 0x1F) == 26) { startx = (float)i+.5; starty = (float)j+.5; } // hardly the place but nobody else really parses the map.

//           drawlight(i,j,rand()%10);
           
/*           drawCube();
           glPopMatrix();*/
       }
 //      printf("\n");
     }
     glEnable(GL_CULL_FACE);
	 glEnable(GL_ALPHA_TEST);
	 //glEnable(GL_DEPTH_TEST);
	 glDisable(GL_BLEND);
}

void init_level()
{
     int i,j;
     object *tobj;
     char buffer[80];
     sprintf(buffer,"snd/game%d.mod",rand()%8);
     if (music == NULL && sfxon) { music = Mix_LoadMUS(buffer);
     Mix_PlayMusic(music,-1);
     }
     
     needRedraw = true;
         needStatusUpdate = true;
     
     topobject = NULL;
     toppart = NULL;
     
     for (i=0; i<64; i++)
     for (j=0; j<64; j++)
     {
           if ((level->tile[i][j] & 0x1F) == 26) { startx = (float)i+.5; starty = (float)j+.5; }
           if ((level->tile[i][j] & 0x1F) == 20) { tobj = (object *)malloc(sizeof(object));
             tobj->type = 10;
             tobj->reg = 1;
             tobj->x = (float)i + .5;
             tobj->y = (float)j + .5;
             tobj->z = (float)(level->tile[i][j] >> 5) +.5;
             tobj->next = topobject;
             tobj->prev = NULL;
             if (topobject != NULL) topobject->prev = tobj;
             topobject = tobj;
           }else if ((level->tile[i][j] & 0x1F) == 10) { tobj = (object *)malloc(sizeof(object));
             tobj->type = 11;
             tobj->reg = 1;
             tobj->x = (float)i + .5;
             tobj->y = (float)j + .5;
             tobj->z = (float)(level->tile[i][j] >> 5) +.5;
             tobj->next = topobject;
             tobj->prev = NULL;
             if (topobject != NULL) topobject->prev = tobj;
             topobject = tobj;
            }else if ((level->tile[i][j] & 0x1F) == 19) { tobj = (object *)malloc(sizeof(object));
             tobj->type = 9;
             tobj->reg = 1;
             tobj->x = (float)i + .5;
             tobj->y = (float)j + .5;
             tobj->z = (float)(level->tile[i][j] >> 5) +.1;
             tobj->next = topobject;
             tobj->prev = NULL;
             if (topobject != NULL) topobject->prev = tobj;
             topobject = tobj;
           } else if ((level->tile[i][j] & 0x1F) == 28 || (level->tile[i][j] & 0x1F) == 29) { tobj = (object *)malloc(sizeof(object));
             tobj->type = 2;
             tobj->x = (float)i + .5;
             tobj->y = (float)j + .5;
             tobj->z = (float)(level->tile[i][j] >> 5) +.5;
             tobj->time = 1.0f;
             tobj->next = topobject;
             tobj->prev = NULL;
             if (topobject != NULL) topobject->prev = tobj;
             topobject = tobj;
             if ((level->tile[i][j] & 0x1F) == 29) tobj->reg = 1.57079632675f; else tobj->reg=0;
           } else if ((level->tile[i][j] & 0x1F) > 11 && (level->tile[i][j] & 0x1F) < 18) {
             tobj = (object *)malloc(sizeof(object));
             tobj->type = (level->tile[i][j] & 0x1F) - 9;
             if (tobj->type > 5) tobj->reg = 0; else tobj->reg = 1;
             tobj->x = (float)i + .5;
             tobj->y = (float)j + .5;
             tobj->z = (float)(level->tile[i][j] >> 5) +.5;
             tobj->next = topobject;
             tobj->prev = NULL;
             if (topobject != NULL)              topobject->prev = tobj;
             topobject = tobj;
           } else if ((level->tile[i][j] & 0x1F) == 6) {
             tobj = (object *)malloc(sizeof(object));
             tobj->type = 12;
             tobj->x = (float)i + .5;
             tobj->y = (float)j + .5;
             tobj->z = (float)(level->tile[i][j] >> 5) +.5;
             tobj->angle = 0;
             tobj->reg = 0;
             tobj->startx = i;
             tobj->starty = j;
             tobj->next = topobject;
             tobj->prev = NULL;
             if (topobject != NULL)              topobject->prev = tobj;
             topobject = tobj;
           } else if ((level->tile[i][j] & 0x1F) == 7) {
             tobj = (object *)malloc(sizeof(object));
             tobj->type = 13;
             tobj->x = (float)i + .5;
             tobj->y = (float)j + .5;
             tobj->z = (float)(level->tile[i][j] >> 5) +.5;
             tobj->startx = i;
             tobj->starty = j;
             tobj->angle = 0;
             tobj->reg = 0;
             tobj->next = topobject;
             tobj->prev = NULL;
             if (topobject != NULL)              topobject->prev = tobj;
             topobject = tobj;
           } else if ((level->tile[i][j] & 0x1F) == 8 || (level->tile[i][j] & 0x1F) == 9 ) {
             tobj = (object *)malloc(sizeof(object));
             tobj->type = 14;
             tobj->startx = i;
             tobj->starty = j;
             tobj->x = (float)i + .5;
             tobj->y = (float)j + .5;
             tobj->z = (float)(level->tile[i][j] >> 5) +.5;
             if ((level->tile[i][j] & 0x1F) == 8)
                 tobj->angle = 0;
             else
                 tobj->angle = 1.57079632675f;
             tobj->reg = 0;
             tobj->next = topobject;
             tobj->prev = NULL;
             if (topobject != NULL)              topobject->prev = tobj;
             topobject = tobj;
            }
     }
           

    ballpx = startx;
    ballpy = starty;
    ballpz = (float)(level->tile[(int)startx][(int)starty] >> 5)+1.0f;
    ballvx = ballvy = ballvz = ballax = ballay = ballaz = 0;
    createcloud(5,ballpx,ballpy,ballpz,0,0,-0.1,0,6,25,1,0,1);


    powerup = 0;
    numkeys = 0;
     playing_fallsound = 0;
     leveldone = 0;

// reset timer
     ticks = SDL_GetTicks();
    secsleft = ticks + (1000 * level->timelimit);
    SDL_WarpMouse(SCREEN_X/2,SCREEN_Y/2);
}

void init_game()
{
     SDL_Surface *tsf;
     char buffer[80];
     int i;
     
     maplist = 0;

	glEnable(GL_DEPTH_TEST);

// Set up the HUD
     bigfont=TTF_OpenFont("Vera.ttf", 20);

     glGenTextures(1, &bgImage);
     tsf = IMG_Load("img/starry.png");
     if (tsf == NULL) { strcpy(ERRMSG,"init_game: Could not load img/starry.png"); gamestate = -1; return; }
     USETEX( bgImage );
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	 glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tsf->w, tsf->h, 0, GL_RGB, GL_UNSIGNED_BYTE, tsf->pixels);
	 SDL_FreeSurface(tsf);
     
     glGenTextures( 1, &hud );
     hudsurf = IMG_Load("img/ui/game/hud.png");
     if (hudsurf == NULL) { strcpy(ERRMSG,"init_game: Could not load img/ui/game/hud.png"); gamestate = -1; return; }
     USETEX ( hud );
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	 glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hudsurf->w, hudsurf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, hudsurf->pixels);

// load levelset
     game_load_levelset(setname);

// load tileset for levelset
     SDL_Surface *ts;
     glGenTextures( 32, tile );
     for (i=0; i<32; i++)
     {
         sprintf(buffer,"img/tiles/%d/%d.png",levelset.tileset,i);
         ts = IMG_Load(buffer);
         if (ts == NULL) { fprintf(stderr,"Couldn't load %s\n",buffer); i = 32; } else {
     USETEX ( tile[i] );
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ts->w, ts->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ts->pixels);
	 SDL_FreeSurface(ts);
         }
     }

// particle images
     glGenTextures( 6, partimgs );
     for (i=0; i<6; i++)
     {
         sprintf(buffer,"img/particle/%d.png",i);
         ts = IMG_Load(buffer);
         if (ts == NULL) { fprintf(stderr,"Couldn't load %s\n",buffer); i = 6; } else {
     USETEX ( partimgs[i] );
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ts->w, ts->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ts->pixels);
	 SDL_FreeSurface(ts);
         }
     }


// couple extra special textures
     glGenTextures( 2, boxtex );
     for (i=0; i<2; i++)
     {
         sprintf(buffer,"img/boxtex/%d.png",i);
         ts = IMG_Load(buffer);
         if (ts == NULL) { fprintf(stderr,"Couldn't load %s\n",buffer); i = 2; } else {
     USETEX ( boxtex[i] );
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_quality);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_quality);
	     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ts->w, ts->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ts->pixels);
	 SDL_FreeSurface(ts);
         }
     }
     
     load_sounds();

     last_used_tex = -1;
     cratemodel = glGenLists(1);
     glNewList(cratemodel,GL_COMPILE);
     glDisable(GL_CULL_FACE);
     USETEX(boxtex[0]);
     drawbox();
     glEnable(GL_CULL_FACE);
     glEndList();

     lockmodel = glGenLists(1);
     glNewList(lockmodel,GL_COMPILE);
     USETEX(boxtex[1]);
     drawbox();
     glEndList();
     
     USETEX(0);

// create the ball
     ballmodel = glGenLists(1);
     glNewList(ballmodel,GL_COMPILE);
//     fghTeapot (7, .5, GL_FILL);
     drawsphere(2,.5,1);
     glEndList();

     spikeball = glGenLists(1);
     glNewList(spikeball,GL_COMPILE);
//     fghTeapot (7, .5, GL_FILL);
     drawspikeball();
     glEndList();

     chipmodel = glGenLists(1);
     glNewList(chipmodel,GL_COMPILE);
     drawchip();
     glEndList();

     dalekmodel = glGenLists(1);
     glNewList(dalekmodel,GL_COMPILE);
     drawdalek();
     glEndList();

     dozermodel = glGenLists(1);
     glNewList(dozermodel,GL_COMPILE);
     drawdozer();
     glEndList();

     fanshot = glGenLists(1);
     glNewList(fanshot,GL_COMPILE);
     drawfanshot();
     glEndList();

     platform = glGenLists(1);
     glNewList(platform,GL_COMPILE);
     drawplatform();
     glEndList();

     keylist = glGenLists(1);
     glNewList(keylist,GL_COMPILE);
     drawkeylist();
     glEndList();
     

// create the powerup FX
     ballfx = glGenLists(1);
     glNewList(ballfx,GL_COMPILE);
	 glDisable(GL_ALPHA_TEST);
	 glEnable(GL_BLEND);
     drawsphere(1,.55,0);
	 glEnable(GL_ALPHA_TEST);
	 glDisable(GL_BLEND);
     glEndList();

// create the level
     init_level();

// start player with 5 lives
     lives = 5;

}

void destroy_level()
{
     object *tobj = topobject;
     while (tobj != NULL)
     {
           tobj = topobject->next;
           free(topobject);
           topobject = tobj;
     }
     
     particle *tpart = toppart;
     while (tpart != NULL)
     {
           tpart = toppart->next;
           free(toppart);
           toppart = tpart;
     }
     
     Mix_HaltMusic(); Mix_FreeMusic(music); music = NULL;
}

void destroy_game()
{
     if (maplist != 0) glDeleteLists(maplist,1);
     destroy_level();

     s_level *tmplvl = levelset.top;
     while (tmplvl != NULL)
     {
           levelset.top = tmplvl->next;
           free(tmplvl);
           tmplvl = levelset.top;
     }
     
     glDeleteLists(dozermodel,1);
     glDeleteLists(chipmodel,1);
     glDeleteLists(dalekmodel,1);
     glDeleteLists(cratemodel,1);
     glDeleteLists(lockmodel,1);
     glDeleteLists(platform,1);
     glDeleteLists(fanshot,1);
     glDeleteLists(keylist,1);
     glDeleteLists(spikeball,1);
     glDeleteLists(ballmodel,1);
     glDeleteLists(ballfx,1);
     glDeleteTextures( 6,partimgs );
     glDeleteTextures( 2,boxtex );
     glDeleteTextures( 32,tile );
     glDeleteTextures( 1,&hud );
     SDL_FreeSurface(hudsurf);
     free_sounds();
     last_used_tex = -1;
}

void draw_game()
{
//     int i, j;
	SDL_Color color={255,255,0};
    SDL_Surface *ts, *ts2;
    SDL_Rect rect;
    char formatted[80];
    static unsigned long oldticks;
    
	glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
#ifdef THREADED
SDL_mutexP(lvlmutex); 
#endif
    if (needRedraw) {
     if (maplist != 0) glDeleteLists(maplist,1);
     USETEX(0); // force texture change in display list
     maplist = glGenLists(1);
     glNewList(maplist,GL_COMPILE);
     drawMap();
     glEndList();
     needRedraw = false;
    }
    
    if ((int)((secsleft - oldticks) / 100) > (int)((secsleft - ticks) / 100)){
    oldticks = ticks;
    needStatusUpdate = true;
    }

    if (needStatusUpdate) {
      needStatusUpdate = false;
	ts2 = SDL_ConvertSurface(hudsurf, hudsurf->format, hudsurf->flags);
	
    sprintf(formatted, "Lvl: %d",level->num);
	ts = TTF_RenderText_Solid(bigfont,formatted,color);
	rect.x = 48;
	rect.y = 32;
	SDL_BlitSurface(ts,NULL,ts2,&rect);
	SDL_FreeSurface(ts);

    sprintf(formatted, "Lives: %d",lives);
	ts = TTF_RenderText_Solid(bigfont,formatted,color);
	rect.x = 36;
	rect.y = 64;
	SDL_BlitSurface(ts,NULL,ts2,&rect);
	SDL_FreeSurface(ts);

    sprintf(formatted, "%0.1f secs",(float)(secsleft - ticks) / 1000.0);
	ts = TTF_RenderText_Solid(bigfont,formatted,color);
	rect.x = 24;
	rect.y = 96;
	SDL_BlitSurface(ts,NULL,ts2,&rect);
	SDL_FreeSurface(ts);

	USETEX ( hud );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ts2->w, ts2->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, ts2->pixels);
	SDL_FreeSurface(ts2);
    }

	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
    glOrtho(0,800,0,600,0,100);

	glMatrixMode( GL_MODELVIEW ) ;
    glLoadIdentity();
    USETEX(bgImage);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex2i(0, 0);
      glTexCoord2f(1, 0);
      glVertex2i(800,0);
      glTexCoord2f(1, 1);
      glVertex2i(800,600);
      glTexCoord2f(0, 1);
      glVertex2i(0,600);
    glEnd();
    
	glEnable(GL_DEPTH_TEST);

	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
    glFrustum(-1,1,-.75,.75,2,100);
    glMatrixMode( GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0,0,-8);
    glRotatef(xrot,0,1,0);
    glRotatef(yrot,1,0,0);

    if (ballpz > -10) glTranslatef(-ballpx,-ballpy,-ballpz);
    else {
         glTranslatef(-ballpx,-ballpy,10);
         if (playing_fallsound != 1) { play_sound(0,ballpx,ballpy); playing_fallsound = 1; }
    }

    glPushMatrix();
    glTranslatef(ballpx,ballpy,ballpz );
    glRotatef(ballry,0,1,0);
    glRotatef(ballrx,1,0,0);
     USETEX(0);
     if (spikeok > ticks + 500)
     {
        glCallList(spikeball);
     }
     if (numkeys > 0)
     {
        glPushMatrix();
        glTranslatef(0,0,.6);
        glCallList(keylist);
        glPopMatrix();
     }
     glCallList(ballmodel);
//     USETEX(0);
//       drawfan();
    glPopMatrix();

#ifdef THREADED
SDL_mutexP(objmutex); 
#endif
    // draw all nontransparents.
     object *tobj = topobject;
     while (tobj != NULL)
     {
           if (tobj->type == 12) {
                  glPushMatrix();
                  glTranslatef(tobj->x, tobj->y, tobj->z);
                  glRotatef(tobj->angle * 57.2957795f,0,0,1);
                  glCallList(chipmodel);
                  glPopMatrix();
           } else if (tobj->type == 13) {
                  glPushMatrix();
                  glTranslatef(tobj->x, tobj->y, tobj->z);
                  glRotatef(tobj->angle * 57.2957795f,0,0,1);
                  glCallList(dalekmodel);
                  glPopMatrix();
           } else if (tobj->type == 14) {
                  glPushMatrix();
                  glTranslatef(tobj->x, tobj->y, tobj->z);
                  glRotatef(tobj->angle * 57.2957795f,0,0,1);
                  glCallList(dozermodel);
                  glPopMatrix();
           } else if (tobj->type == 10) {
                          if (tobj->reg == 1) {
                  glPushMatrix();
                  glTranslatef(tobj->x, tobj->y, tobj->z);
                  glCallList(lockmodel);
                  glPopMatrix();
                  }
           }
           else if (tobj->type == 11) {
                          if (tobj->reg == 1) {
                  glPushMatrix();
                  glTranslatef(tobj->x, tobj->y, tobj->z);
                  glCallList(cratemodel);
                  glPopMatrix();
                  }
           }
           else if (tobj->type == 9) {
                          if (tobj->reg == 1) {
                  glPushMatrix();
                  glTranslatef(tobj->x, tobj->y, tobj->z);
                  glCallList(keylist);
                  glPopMatrix();
                  glColor3f(1,1,1);
                  }
           }
           else if (tobj->type > 2) {
             if (tobj->reg == 1) {
                    if (tobj->type == 3 || tobj->type == 6) glColor3f(1,0,0);
                    else if (tobj->type == 4 || tobj->type == 7) glColor3f(1,1,0);
                    else glColor3f(0,0,1);
                  glPushMatrix();
                  if (tobj->type > 5) {
                  glTranslatef(tobj->x, tobj->y, tobj->z); } else {
                  glTranslatef(tobj->x, tobj->y, tobj->z-.3);
                  glScalef(.3875,.3875,.3875); }
                  glCallList(platform);
                  glPopMatrix();
                  glColor3f(1,1,1);
                  }
           }
           tobj = tobj->next;
     }
#ifdef THREADED
SDL_mutexV(objmutex); 
#endif

      glCallList(maplist);

    if (powerup != 0) {
    glPushMatrix();
    glTranslatef(ballpx,ballpy,ballpz );
    if (powerup == 1)
    {
      glColor4f(1,1,0,.25);
      glCallList(ballfx);
      glColor4f(1,1,1,1);
    } else if (powerup == 2) {
      glColor4f(0,0,1,.25);
      glCallList(ballfx);
      glColor4f(1,1,1,1);
    } else if (powerup == 3) {
      glColor4f(1,0,0,.25);
      glCallList(ballfx);
      glColor4f(1,1,1,1);
    }
    glPopMatrix();
    }

#ifdef THREADED
SDL_mutexP(objmutex); 
#endif
    // we do just transparent objects here.
     tobj = topobject;
     while (tobj != NULL)
     {
           if (tobj->type == 1) {
                  glPushMatrix();
                  glTranslatef(tobj->x, tobj->y, tobj->z);
                  glRotatef(tobj->angle * 57.2957795f,0,0,1);
                  glCallList(fanshot);
                  glPopMatrix();
           }
           tobj = tobj->next;
     }
#ifdef THREADED
SDL_mutexV(objmutex); 
#endif
     
	 glEnable(GL_BLEND);
#ifdef THREADED
SDL_mutexP(partmutex); 
#endif
     // draw all particles.
     particle *tpart = toppart;
     while (tpart != NULL)
     {
           USETEX(partimgs[tpart->type]);
           glPushMatrix();
           glTranslatef(tpart->x, tpart->y, tpart->z);
           glRotatef(tpart->ang,0,0,1);
           if (tpart->life < 1) glColor4f(1,1,1,tpart->life);
           glBegin(GL_QUADS);
             glTexCoord2f(0,0);
             glVertex2f(-.2,-.2);
             glTexCoord2f(1,0);
             glVertex2f(.2,-.2);
             glTexCoord2f(1,1);
             glVertex2f(.2,.2);
             glTexCoord2f(0,1);
             glVertex2f(-.2,.2);
           glEnd();
           if (tpart->life < 1) glColor4f(1,1,1,1);
           glPopMatrix();
           tpart = tpart->next;
     }
#ifdef THREADED
SDL_mutexV(partmutex); 
#endif
	 glDisable(GL_BLEND);

#ifdef THREADED
SDL_mutexV(lvlmutex);
#endif

    glDepthMask(GL_TRUE);

	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
    glOrtho(0,800,0,600,0,100);

	glMatrixMode( GL_MODELVIEW ) ;
    glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
    USETEX ( hud );
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex2i(672, 128);
      glTexCoord2f(0, 1);
      glVertex2i(672,0);
      glTexCoord2f(1, 1);
      glVertex2i(800,0);
      glTexCoord2f(1, 0);
      glVertex2i(800,128);
    glEnd();

    USETEX ( cursor );
    glColor3f(1,1,1);
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
    
/*    GLenum glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
          switch (glErr) {
                 case GL_INVALID_ENUM:
                      fprintf(stderr,"gl error: GL_INVALID_ENUM\n");
                      break;
                 case GL_INVALID_VALUE:
                      fprintf(stderr,"gl error: GL_INVALID_VALUE\n");
                      break;
                 case GL_INVALID_OPERATION:
                      fprintf(stderr,"gl error: GL_INVALID_OPERATION\n");
                      break;
                 case GL_STACK_OVERFLOW:
                      fprintf(stderr,"gl error: GL_STACK_OVERFLOW\n");
                      break;
                 case GL_STACK_UNDERFLOW:
                      fprintf(stderr,"gl error: GL_STACK_UNDERFLOW\n");
                      break;
                 case GL_OUT_OF_MEMORY:
                      fprintf(stderr,"gl error: GL_OUT_OF_MEMORY\n");
                      break;
                      }
          glErr = glGetError();
    }*/
//    fprintf(stderr,"mark\n");
//    SDL_mutexV ( mutex );
    SDL_Delay(1);
    
}

bool handle_game()
{
     static int xchg=0, ychg=0, usingkeys = 0;
    bool retval = false;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_MOUSEBUTTONUP:
                 usingkeys = 0;
                 mx = event.motion.x * 800 / SCREEN_X;
                 my = event.motion.y * 600 / SCREEN_Y;
                 if (powerup == 3 && jumpok) { jumpok = 0; ballvz = 20; play_sound(7,ballpx,ballpy); }
                 else if (powerup == 2 && revok < ticks) { revok = ticks + 1000; ballvx *= 2; ballvy *= 2; play_sound(8,ballpx,ballpy); }
                 else if (powerup == 1 && spikeok < ticks-1000) { spikeok = ticks + 1000; play_sound(9,ballpx,ballpy); }
                 break;
            case SDL_MOUSEMOTION:
                 usingkeys = 0;
                 mx = (int)((float)event.motion.x * 800.0f / (float)SCREEN_X);
                 my = (int)((float)event.motion.y * 600.0f / (float)SCREEN_Y);
                 xrot = 20*((float)mx - 400) / 800;
                 yrot = 20*((float)my - 300) / 600;
//                 xrot = 15*((float)mx - (SCREEN_X/2)) / SCREEN_X;
//                 yrot = 15*((float)my - (SCREEN_Y/2)) / SCREEN_Y;
                 break;
            case SDL_KEYUP:
                 usingkeys = 1;
                 if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN)
                   ychg = 0;
                 else if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT)
                   xchg = 0;
                 else if(event.key.keysym.sym == SDLK_ESCAPE)
                       gamestate = 0;
                 else
                   if (powerup == 3 && jumpok) { jumpok = 0; ballvz = 20; play_sound(7,ballpx,ballpy); }
                 else if (powerup == 2 && revok < ticks) { revok = ticks + 1000; ballvx *= 2; ballvy *= 2; play_sound(8,ballpx,ballpy); }
                 else if (powerup == 1 && spikeok < ticks-1000) { spikeok = ticks + 1000; play_sound(9,ballpx,ballpy); }
                   break;
            case SDL_KEYDOWN:
                 usingkeys = 1;
                 if (event.key.keysym.sym == SDLK_UP)
                   ychg = -1;
                 if (event.key.keysym.sym == SDLK_DOWN)
                   ychg = 1;
                 if (event.key.keysym.sym == SDLK_LEFT)
                   xchg = -1;
                 if (event.key.keysym.sym == SDLK_RIGHT)
                   xchg = 1;
                 break;
            case SDL_QUIT:
                 retval = true;
                 break;
            default:
                    break;
        }
    }
    if (usingkeys) {
    if (ychg == 0 && yrot < -1) yrot++;
    else if (ychg == 0 && yrot > 1) yrot--;
    else if (ychg == 0) yrot=0;
    else yrot += ychg;
    if (xchg == 0 && xrot < -1) xrot++;
    else if (xchg == 0 && xrot > 1) xrot--;
    else if (xchg == 0) xrot=0;
    else xrot += xchg;
    }

    #ifdef UNIMOTION
    double x,y,z;
    int ok;
    ok = read_sms_real(sms_type, &x, &y, &z);
    if (!ok) printf("Error with read_sms_real!\n");
    if (fabs(z) > .5) {
          if (powerup == 3 && jumpok) { jumpok = 0; ballvz = 20; play_sound(7,ballpx,ballpy); }
          else if (powerup == 2 && revok < ticks) { revok = ticks + 1000; ballvx *= 2; ballvy *= 2; play_sound(8,ballpx,ballpy); }
          else if (powerup == 1 && spikeok < ticks-1000) { spikeok = ticks + 1000; play_sound(9,ballpx,ballpy); }
    }
    
    xrot = x * 15;
    yrot = y * 15;
    #endif
    
    if (yrot < -10) yrot = -10;
    if (yrot > 10) yrot = 10;
    if (xrot < -10) xrot = -10;
    if (xrot > 10) xrot = 10;

//    #ifdef UNIMOTION
    mx = (int)(((xrot + 10.0f) / 20.0f) * 800.0f);
    my = (int)(((yrot + 10.0f) / 20.0f) * 600.0f);
//    #endif

    return retval;
}

/* two choices of game simulation: this one does 1 update/X microseconds */
int simulate_game(void *)
{
    int i,j;
    double delta;
    double gravity = 35;
    object *tobj, *tobj2;
    particle *tpart, *tpart2;

#ifdef THREADED
    while (gamestate == 10) {
#endif
//          if (handle_game()) { gamestate=0; return 0; }
     delta = (double)(SDL_GetTicks() - ticks) / 1000;

     ticks = SDL_GetTicks();
     jumpok = 0;

     if (leveldone == 0) {

     if (secsleft < ticks) {
//         powerup = 0;
         secsleft = ticks + (1000 * level->timelimit);

tobj2 = topobject;
     while (tobj2 != NULL)
     { if (tobj2->type > 11 && tobj2->type < 15) { tobj2->x = (float)tobj2->startx + .5; tobj2->y = (float)tobj2->starty + .5; tobj2->z = (float)(level->tile[tobj2->startx][tobj2->starty] >> 5);} 
     tobj2 = tobj2->next;}
         lives--;
    SDL_WarpMouse(SCREEN_X/2,SCREEN_Y/2);
         needStatusUpdate = true;
            if (lives < 0) { gamestate = 0; } else {
            ballpx = startx;
            ballpy = starty;
            ballpz = (float)(level->tile[(int)startx][(int)starty] >> 5)+1.0f;
            ballvx = ballvy = ballvz = ballax = ballay = ballaz = 0;
            createcloud(5,ballpx,ballpy,ballpz,0,0,-0.1,0,6,25,1,0,1);
            }
         play_sound(6,ballpx,ballpy);
     } // Out of time!
     
     // acceleration stuff.
     ballax = (sin (xrot * 0.0174532925) * gravity);
     ballay = -(sin (yrot * 0.0174532925) * gravity);
     ballaz = -gravity;

     // adjust velocity based on accel
     ballvx += ballax*delta;
     ballvy += ballay*delta;
     ballvz += ballaz*delta;

#ifdef THREADED
                   SDL_mutexP(partmutex);
#endif
      tpart = toppart;
      while (tpart != NULL)
      {
            tpart->life -= delta;
            tpart->x += tpart->vx * delta;
            tpart->y += tpart->vy * delta;
            tpart->vz += tpart->az * delta;
            tpart->z += tpart->vz * delta;
            tpart->ang += tpart->rot;
            if (tpart->life <= 0 || tpart->x < 0 || tpart->x >= 64 || tpart->y < 0 || tpart->y >= 64 || tpart->z < -10 ||
              ((level->tile[(int)tpart->x][(int)tpart->y] >> 5) > 0 && (level->tile[(int)tpart->x][(int)tpart->y] >> 5) > tpart->z))
            {
                         if (tpart->prev == NULL)
                         {
                             toppart = tpart->next;
                         } else {
                             tpart->prev->next = tpart->next;
                         }
                         if (tpart->next != NULL)
                           tpart->next->prev = tpart->prev;
                         tpart2 = tpart->next;
                         free(tpart);
                         tpart = tpart2;
            } else {
                tpart = tpart->next;
            }
      }
#ifdef THREADED
     SDL_mutexV(partmutex);
#endif

#ifdef THREADED
    SDL_mutexP(objmutex);
#endif
     tobj = topobject;
     while (tobj != NULL)
     {
           switch (tobj->type)
           {
                  case 1: // moving fanshot
                      tobj->x += 4*cos(tobj->angle) * delta;
                      tobj->y += 4*sin(tobj->angle) * delta;
                      // check collision with ball
                      i = 0;
                      if (tobj->x - .5 < (ballpx-(ballvx*delta)) && tobj->x + .5 > (ballpx-(ballvx*delta)) &&
                      tobj->y - .5 < (ballpy-(ballvy*delta)) && tobj->y + .5 > (ballpy-(ballvy*delta)) &&
                      tobj->z - .5 < (ballpz-(ballvz*delta)) && tobj->z + .5 > (ballpz-(ballvz*delta)))
                      {
                              i = 1;
                              ballvx += cos(tobj->angle);
                              ballvy += sin(tobj->angle);
                      }
                      if (tobj->x < 0.5 || tobj->x > 63.5 || tobj->y < 0.5 || tobj->y > 63.5
                       || level->tile[(int)(tobj->x)][(int)(tobj->y)] >> 5 > tobj->z || i == 1)
                      {
                   createcloud(0,tobj->x, tobj->y, tobj->z,.5,.5,0,0,6,15,1,0,1); // fanshots puff away

                         if (tobj->prev == NULL)
                         {
                             topobject = tobj->next;
                         } else {
                             tobj->prev->next = tobj->next;
                         }
                         if (tobj->next != NULL)
                           tobj->next->prev = tobj->prev;
                         tobj2 = tobj->next;
                         free(tobj);
                         tobj = tobj2;
                      } else {
                         tobj = tobj->next;
                      }                             
                      break;
                  case 2: // fan
                       tobj->time -= delta;
                       if (tobj->time <=0)
                       {
                           play_sound(13,tobj->x,tobj->y);
                           tobj->time = 0.25;
                           tobj2 = (object *)malloc(sizeof(object));
                           tobj2->next = topobject;
                           tobj2->prev = NULL;
                           topobject->prev = tobj2;
                           topobject = tobj2;
                           tobj2->x = tobj->x;
                           tobj2->y = tobj->y;
                           tobj2->z = tobj->z;
                           tobj2->angle = 0 + tobj->reg;
                           tobj2->type = 1;

                           tobj2 = (object *)malloc(sizeof(object));
                           tobj2->next = topobject;
                           tobj2->prev = NULL;
                           topobject->prev = tobj2;
                           topobject = tobj2;
                           tobj2->x = tobj->x;
                           tobj2->y = tobj->y;
                           tobj2->z = tobj->z;
                           tobj2->angle = 3.1415926535f + tobj->reg;
                           tobj2->type = 1;
                       }
                       tobj = tobj->next;
                       break;
                  case 12:
                       if (tobj->x > .5 && tobj->x < 63.5 && tobj->y > .5 && tobj->y < 63.5 &&
                          (level->tile[(int)tobj->x][(int)tobj->y] >> 5) > 0) {

                       if (xrot != 0 || yrot != 0) {
                     tobj->angle = atan2(sin (yrot * 0.0174532925),-sin (xrot * 0.0174532925));
                     tobj->reg = (sin (xrot * 0.0174532925) * gravity * delta) + (3*cos(tobj->angle) * delta);
                     if (tobj->reg > 0 && xrot < 0) tobj->reg = 0;
                     else if (tobj->reg < 0 && xrot > 0) tobj->reg = 0;
                       if ((float)(level->tile[(int)(tobj->x + tobj->reg)][(int)tobj->y] >> 5) + .5 <= tobj->z)
                          tobj->x += tobj->reg;

                     tobj->reg = (-sin (yrot * 0.0174532925) * gravity * delta) + (3*sin(tobj->angle) * delta);
                     if (tobj->reg < 0 && yrot < 0) tobj->reg = 0;
                     else if (tobj->reg > 0 && yrot > 0) tobj->reg = 0;
                       if ((float)(level->tile[(int)tobj->x][(int)(tobj->y + tobj->reg)] >> 5) + .5 <= tobj->z)
                          tobj->y += tobj->reg;

                          }
                          
                          tobj->reg = 0;
                       tobj->z -= delta * gravity;
                       
                       if ((float)(level->tile[(int)tobj->x][(int)tobj->y] >> 5) + .5 > (int)tobj->z && (level->tile[(int)tobj->x][(int)tobj->y] >> 5)!= 0)
                       {
                         tobj->z = .5 + (float)(level->tile[(int)tobj->x][(int)tobj->y] >> 5);
                         }
     
                       // collide with player!
                      if (tobj->x - .75 < (ballpx-(ballvx*delta)) && tobj->x + .75 > (ballpx-(ballvx*delta)) &&
                      tobj->y - .75 < (ballpy-(ballvy*delta)) && tobj->y + .75 > (ballpy-(ballvy*delta)) &&
                      tobj->z - .5 < (ballpz-(ballvz*delta)) && tobj->z + .5 > (ballpz-(ballvz*delta)))
                      {
                              play_sound(17,ballpx,ballpy);
                      createcloud(2,tobj->x, tobj->y, tobj->z,1,1,0,-gravity / 10,6,100,1,0,2);
            lives--;
    SDL_WarpMouse(SCREEN_X/2,SCREEN_Y/2);
tobj2 = topobject;
     while (tobj2 != NULL)
     { if (tobj2->type > 11 && tobj2->type < 15) { tobj2->x = (float)tobj2->startx + .5; tobj2->y = (float)tobj2->starty + .5; tobj2->z = (float)(level->tile[tobj2->startx][tobj2->starty] >> 5);} 
     tobj2 = tobj2->next;}
         needStatusUpdate = true;
//            powerup = 0;
            secsleft = ticks + (1000 * level->timelimit);
            playing_fallsound = 0;
            if (lives < 0) { gamestate = 0; } else {
            ballpx = startx;
            ballpy = starty;
            ballpz = (float)(level->tile[(int)startx][(int)starty] >> 5)+1.0f;
            ballvx = ballvy = ballvz = ballax = ballay = ballaz = 0;
            createcloud(5,ballpx,ballpy,ballpz,0,0,-0.1,0,6,25,1,0,1);
            }
                      }
                       tobj = tobj->next;
                      } else {
                       tobj->z -= delta * gravity * .5;
                       if (tobj->z < -10 && tobj->reg == 0)
                       {
                         play_sound(2,tobj->x,tobj->y);
                         tobj->reg = 1;
                       }
                       if (tobj->z < -40) {
                         if (tobj->prev == NULL)
                         {
                             topobject = tobj->next;
                         } else {
                             tobj->prev->next = tobj->next;
                         }
                         if (tobj->next != NULL)
                           tobj->next->prev = tobj->prev;
                         tobj2 = tobj->next;
                         free(tobj);
                         tobj = tobj2;
                       } else {
                             tobj = tobj->next;
                       }
                       }
                       break;
                  case 13:
                       tobj->angle = atan2((double)(tobj->y - ballpy), (double)(tobj->x - ballpx));
                       if (tobj->x > .5 && tobj->x < 63.5 && tobj->y > .5 && tobj->y < 63.5 &&
                          (level->tile[(int)tobj->x][(int)tobj->y] >> 5) > 0) {
                       tobj->reg = tobj->x - (cos(tobj->angle) * delta);
                       if ((float)(level->tile[(int)tobj->reg][(int)tobj->y] >> 5) + .5 <= tobj->z)
                          tobj->x = tobj->reg;

                       tobj->reg = tobj->y - (sin(tobj->angle) * delta);
                       if ((float)(level->tile[(int)tobj->x][(int)tobj->reg] >> 5) + .5 <= tobj->z)
                          tobj->y = tobj->reg;
                          
                          tobj->reg = 0;

                       tobj->z -= delta * gravity;
                       
                       if ((float)(level->tile[(int)tobj->x][(int)tobj->y] >> 5) + .5 > (int)tobj->z)
                       {
                         tobj->z = (level->tile[(int)tobj->x][(int)tobj->y] >> 5) + .5;
                         }

//                       tobj->y += sin(tobj->angle) * delta;
                       // collide with player!
                      if (tobj->x - .75 < (ballpx-(ballvx*delta)) && tobj->x + .75 > (ballpx-(ballvx*delta)) &&
                      tobj->y - .75 < (ballpy-(ballvy*delta)) && tobj->y + .75 > (ballpy-(ballvy*delta)) &&
                      tobj->z - .5 < (ballpz-(ballvz*delta)) && tobj->z + .5 > (ballpz-(ballvz*delta)))
                      {
                              play_sound(17,ballpx,ballpy);
                      createcloud(2,tobj->x, tobj->y, tobj->z,1,1,0,-gravity / 10,6,100,1,0,2);
            lives--;
    SDL_WarpMouse(SCREEN_X/2,SCREEN_Y/2);
tobj2 = topobject;
     while (tobj2 != NULL)
     { if (tobj2->type > 11 && tobj2->type < 15) { tobj2->x = (float)tobj2->startx + .5; tobj2->y = (float)tobj2->starty + .5; tobj2->z = (float)(level->tile[tobj2->startx][tobj2->starty] >> 5);} 
     tobj2 = tobj2->next;}
         needStatusUpdate = true;
//            powerup = 0;
            secsleft = ticks + (1000 * level->timelimit);
            playing_fallsound = 0;
            if (lives < 0) { gamestate = 0; } else {
            ballpx = startx;
            ballpy = starty;
            ballpz = (float)(level->tile[(int)startx][(int)starty] >> 5)+1.0f;
            ballvx = ballvy = ballvz = ballax = ballay = ballaz = 0;
            createcloud(5,ballpx,ballpy,ballpz,0,0,-0.1,0,6,25,1,0,1);
            }
                      }
                       tobj = tobj->next;
                      } else {
                       tobj->z -= delta * gravity * .5;
                       if (tobj->z < -10 && tobj->reg == 0)
                       {
                         play_sound(18,tobj->x,tobj->y);
                         tobj->reg = 1;
                       }
                       if (tobj->z < -40) {
                         if (tobj->prev == NULL)
                         {
                             topobject = tobj->next;
                         } else {
                             tobj->prev->next = tobj->next;
                         }
                         if (tobj->next != NULL)
                           tobj->next->prev = tobj->prev;
                         tobj2 = tobj->next;
                         free(tobj);
                         tobj = tobj2;
                       } else {
                             tobj = tobj->next;
                       }
                       }
                      
                       break;
                  case 14:
                       if ((level->tile[(int)(tobj->x + (delta * 2*cos(tobj->angle)))][(int)(tobj->y + (delta * 2*sin(tobj->angle)))] >> 5) == (int)tobj->z)
                       {
                          tobj->x += 2*cos(tobj->angle) * delta;
                          tobj->y += 2*sin(tobj->angle) * delta;
                       } else {
                          tobj->angle += 3.141592653f;
                          if (tobj->angle > 6.2) tobj->angle-=6.283185306f;
                       }
                       // collide with player!
                      if (tobj->x - .75 < (ballpx-(ballvx*delta)) && tobj->x + .75 > (ballpx-(ballvx*delta)) &&
                      tobj->y - .75 < (ballpy-(ballvy*delta)) && tobj->y + .75 > (ballpy-(ballvy*delta)) &&
                      tobj->z - .5 < (ballpz-(ballvz*delta)) && tobj->z + .5 > (ballpz-(ballvz*delta)))
                      {
                              play_sound(17,ballpx,ballpy);
                      createcloud(2,tobj->x, tobj->y, tobj->z,1,1,0,-gravity / 10,6,100,1,0,2);
            lives--;
    SDL_WarpMouse(SCREEN_X/2,SCREEN_Y/2);
tobj2 = topobject;
     while (tobj2 != NULL)
     { if (tobj2->type > 11 && tobj2->type < 15) { tobj2->x = (float)tobj2->startx + .5; tobj2->y = (float)tobj2->starty + .5; tobj2->z = (float)(level->tile[tobj2->startx][tobj2->starty] >> 5);} 
     tobj2 = tobj2->next;}
         needStatusUpdate = true;
//            powerup = 0;
            secsleft = ticks + (1000 * level->timelimit);
            playing_fallsound = 0;
            if (lives < 0) { gamestate = 0; } else {
            ballpx = startx;
            ballpy = starty;
            ballpz = (float)(level->tile[(int)startx][(int)starty] >> 5)+1.0f;
            ballvx = ballvy = ballvz = ballax = ballay = ballaz = 0;
            createcloud(5,ballpx,ballpy,ballpz,0,0,-0.1,0,6,25,1,0,1);
            }
                      }
                       tobj = tobj->next;
                       break;
                       
                  default:
                          tobj = tobj->next;
                          break; // unknown object...
           }
     }
#ifdef THREADED
     SDL_mutexV(objmutex);
#endif

     if (ballpx >= 0 && ballpx < 64 && ballpy >= 0 && ballpy < 64 && ballpz > .9) {

     if (ballvx < 0 && ballpz + (ballvz*delta) + .5 < (level->tile[(int)(ballpx-(ballvx*delta)-.5)][(int)(ballpy-(delta*ballvy))] >> 5))
     {
                ballvx = fabs(ballvx)/2;//+(10*delta);//.25;
                play_sound(3,ballpx,ballpy, (int)min(128,(int)20*ballvx));
     }
     else if (ballvx > 0 && ballpz + (ballvz*delta) + .5 < (level->tile[(int)(ballpx-(ballvx*delta)+.5)][(int)(ballpy-(delta*ballvy))] >> 5))
     {
                ballvx = -fabs(ballvx)/2;//-(10*delta);//.25;
                play_sound(3,ballpx,ballpy, (int)min(128,(int)20*fabs(ballvx)));
     }

     if (ballvy < 0 && ballpz + (ballvz*delta) + .5 < (level->tile[(int)(ballpx-(ballvx*delta))][(int)(ballpy-(delta*ballvy)-.5)] >> 5))
     {
                ballvy = fabs(ballvy)/2;//+(10*delta);//.25;
                play_sound(3,ballpx,ballpy, (int)min(128,(int)20*ballvy));
     }

     else if (ballvy > 0 && ballpz + (ballvz*delta) + .5 < (level->tile[(int)(ballpx-(ballvx*delta))][(int)(ballpy-(delta*ballvy)+.5)] >> 5))
     {
                ballvy = -fabs(ballvy)/2;//-(10*delta);//.25;
                play_sound(3,ballpx,ballpy, (int)min(128,(int)20*fabs(ballvy)));
     }

     if (ballpz + (ballvz*delta) <= 1.5+(level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] >> 5))
     {
         // ball may hit object...
         switch (level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] & 0x1F)
         {
              case 10: // smashy crate
                   tobj = topobject;
                   while (tobj != NULL)
                   {
                         if (tobj->type == 11 && (int)tobj->x == (int)(ballpx-(delta*ballvx)) && (int)tobj->y == (int)(ballpy-(delta*ballvy))) {
                            if (tobj->reg == 0) tobj = NULL; else {
                            if ((ballpz + (ballvz*delta) < 1+(level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] >> 5) && sqrt((ballvx * ballvx) + (ballvy * ballvy)) > 3) || ballvz < -10)
                            {
                               tobj->reg = 0;
                               play_sound(1,tobj->x,tobj->y);
                   createcloud(1,tobj->x, tobj->y, tobj->z,3,3,5,-gravity/2,6,25,0,1,3);
                            } else {
                               
                            if (ballpz + (ballvz*delta) > 1+(level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] >> 5)){
                            ballpz = 1.5+(level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] >> 5);
                            ballvz = 0;
                            }else {
                            if ((ballvx < 0 && (ballpx-(ballvx*delta)) - (int)(ballpx-(ballvx*delta)) > .75) || (ballvx > 0 && (ballpx-(ballvx*delta)) - (int)(ballpx-(ballvx*delta)) < .25))
                            ballvx = -ballvx;
                            if ((ballvy < 0 && (ballpy-(ballvy*delta)) - (int)(ballpy-(ballvy*delta)) > .75) || (ballvy > 0 && (ballpy-(ballvy*delta)) - (int)(ballpy-(ballvy*delta)) < .25))
                            ballvy = -ballvy;
                            }
                            tobj = NULL;
                            }}
                         } else {
                            tobj = tobj->next;
                         }
                   }
                   break;

              case 12:
              case 13:
              case 14:
                   tobj = topobject;
                   while (tobj != NULL)
                   {
                         if (tobj->type > 2 && tobj->type < 6 && tobj->reg == 1 && (int)tobj->x == (int)(ballpx-(delta*ballvx)) && (int)tobj->y == (int)(ballpy-(delta*ballvy))) {
                            tobj->reg = 0;
                            play_sound(14,tobj->x,tobj->y);
                   createcloud(0,tobj->x, tobj->y, tobj->z,.5,.5,0,0,6,15,1,0,1);
                            tobj2 = topobject;
                            while (tobj2 != NULL)
                            {
                                  if (tobj2->type == tobj->type + 3 && tobj2->reg == 0) {
                                      tobj2->reg = 1;
                                      level->tile[(int)tobj2->x][(int)tobj2->y] = (level->tile[(int)tobj2->x][(int)tobj2->y] & 0x1F) + (((level->tile[(int)tobj2->x][(int)tobj2->y] >> 5) + 1) << 5);
                                  }
                                  tobj2 = tobj2->next;
                            }
                            tobj = NULL;
                         } else {
                            tobj = tobj->next;
                         }
                   }
                   break;
              case 18:
                   ballvz += gravity * 1.5 * delta;
                   break;
              case 19:
                   tobj = topobject;
                   while (tobj != NULL)
                   {
                         if (tobj->type == 9 && tobj->reg == 1 && (int)tobj->x == (int)(ballpx-(delta*ballvx)) && (int)tobj->y == (int)(ballpy-(delta*ballvy))) {
                            tobj->reg = 0;
                            play_sound(15,tobj->x,tobj->y);
                   createcloud(0,tobj->x, tobj->y, tobj->z,.5,.5,0,0,6,15,1,0,1);
                            numkeys++;
                            tobj = NULL;
                         } else {
                            tobj = tobj->next;
                         }
                   }
                   break;
              case 20: // locked door: got a key?
                   tobj = topobject;
                   while (tobj != NULL)
                   {
                         if (tobj->type == 10 && (int)tobj->x == (int)(ballpx-(delta*ballvx)) && (int)tobj->y == (int)(ballpy-(delta*ballvy))) {
                            if (tobj->reg == 0) tobj = NULL; else {
                            if (numkeys > 0)
                            {
                               tobj->reg = 0;
                               numkeys--;
                               play_sound(16,tobj->x,tobj->y);
                   createcloud(1,tobj->x, tobj->y, tobj->z,3,3,5,-gravity/2,6,25,0,1,3);
                            } else {
                               
                            if (ballpz + (ballvz*delta) > 1+(level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] >> 5)){
                            ballpz = 1.5+(level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] >> 5);
                            ballvz = 0;
                            }else {
                            if ((ballvx < 0 && (ballpx-(ballvx*delta)) - (int)(ballpx-(ballvx*delta)) > .75) || (ballvx > 0 && (ballpx-(ballvx*delta)) - (int)(ballpx-(ballvx*delta)) < .25))
                            ballvx = -ballvx;
                            if ((ballvy < 0 && (ballpy-(ballvy*delta)) - (int)(ballpy-(ballvy*delta)) > .75) || (ballvy > 0 && (ballpy-(ballvy*delta)) - (int)(ballpy-(ballvy*delta)) < .25))
                            ballvy = -ballvy;
                            }
                            tobj = NULL;
                            }}
                         } else {
                            tobj = tobj->next;
                         }
                   }
                   break;
              case 30:
                   play_sound(11,(int)(ballpx-(delta*ballvx))+.5,(int)(ballpy-(delta*ballvy))+.5);
                   if ((ballvx < 0 && (ballpx-(ballvx*delta)) - (int)(ballpx-(ballvx*delta)) > .75) || (ballvx > 0 && (ballpx-(ballvx*delta)) - (int)(ballpx-(ballvx*delta)) < .25))
                   ballvx *= -2;
                   if ((ballvy < 0 && (ballpy-(ballvy*delta)) - (int)(ballpy-(ballvy*delta)) > .75) || (ballvy > 0 && (ballpy-(ballvy*delta)) - (int)(ballpy-(ballvy*delta)) < .25))
                   ballvy *= -2;
                   break;
              case 31:
                   break;
              default:
                   break;
         }

     if (ballpz + (ballvz*delta) <= .5+(level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] >> 5))
     {
         // ball hit ground - what now?
         ballpz = .5+(level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] >> 5);
         if (spikeok > ticks + 500) {ballvz = 0; ballvx = 0; ballvy = 0; } else 
         if (ballvz < -(2*delta*gravity)) { ballvz = fabs(ballvz/2); play_sound(3,ballpx,ballpy, (int)min(128,(int)20*ballvz)); }else
         ballvz = 0;
         jumpok = 1;

         int tiletype = level->tile[(int)(ballpx-(delta*ballvx))][(int)(ballpy-(delta*ballvy))] & 0x1F;
         switch (tiletype)
         {
                case 21: // teleporter
                   createcloud(3,ballpx, ballpy, ballpz+2.0,.5,.5,-.1,0,6,25,1,0,2);
                  for (i=0; i<64; i++)
                    for (j=0; j<64; j++)
                      if ((i != (int)(ballpx-(delta*ballvx)) || j != (int)(ballpy-(delta*ballvy))) && (level->tile[i][j] & 0x1F) == 21)
                      {
                            ballpx = i + .5;
                            ballpy = j + .5;
                            ballpz = (level->tile[i][j] >> 5) + .5;
                   createcloud(3,ballpx, ballpy, ballpz+2.0,.5,.5,-.1,0,6,25,1,0,2);
                            i=64;
                            j=64;
                      }
                  ballvx = 0;
                  ballvy = 0;
                  ballvz = 15;
                  ballax = 0;
                  ballay = 0;
                  ballaz = 0;
                  play_sound(10,ballpx,ballpy);
                  break;
                case 22: // jump pad
                  ballvz = 30;
                  play_sound(4,ballpx,ballpy);
                   createcloud(0,ballpx, ballpy, ballpz,.5,.5,.1,0,6,25,1,0,1);
                  break;
                case 23: // spikes powerup
                  if (powerup != 1) {
                  powerup = 1;
                  play_sound(12,ballpx,ballpy);
                   createcloud(4,ballpx, ballpy, ballpz,.5,.5,0,1,6,25,1,0,1);
                  }
                  break;
                case 24: // speed boost powerup
                  if (powerup != 2) {
                  powerup = 2;
                  play_sound(12,ballpx,ballpy);
                   createcloud(4,ballpx, ballpy, ballpz,.5,.5,0,1,6,25,1,0,1);
                  }
                  break;
                case 25: // jump powerup
                  if (powerup != 3) {
                  powerup = 3;
                  play_sound(12,ballpx,ballpy);
                   createcloud(4,ballpx, ballpy, ballpz,.5,.5,0,1,6,25,1,0,1);
                  }
                  break;
                case 27: // level exit!
                  lives++;
                  play_sound(5,ballpx,ballpy);
                   createcloud(5,ballpx, ballpy, ballpz,.5,.5,0,1,6,50,1,0,1);
                  leveldone = 1;
                  secsleft = ticks + 3000;
                  mx = 400;
                  my = 300;
                  break;
                default:
                  break;
         }
         switch (tiletype)
         {
                case 4:   // ice, no friction
                  break;
                case 5: // glue, make sure 2x friction
                  if (ballvx < delta) 
                    ballvx += delta;
                  else if (ballvx > delta)
                    ballvx -= delta;
                  else
                    ballvx = 0;

                  if (ballvy < delta) 
                    ballvy += delta;
                  else if (ballvy > delta)
                    ballvy -= delta;
                  else
                    ballvy = 0;
                default: // regular friction
                  if (ballvx < delta) 
                    ballvx += delta;
                  else if (ballvx > delta)
                    ballvx -= delta;
                  else
                    ballvx = 0;

                  if (ballvy < delta) 
                    ballvy += delta;
                  else if (ballvy > delta)
                    ballvy -= delta;
                  else
                    ballvy = 0;
         }
     }
     }

     } else if (ballpz < -40) {
tobj2 = topobject;
     while (tobj2 != NULL)
     { if (tobj2->type > 11 && tobj2->type < 15) { tobj2->x = (float)tobj2->startx + .5; tobj2->y = (float)tobj2->starty + .5; tobj2->z = (float)(level->tile[tobj2->startx][tobj2->starty] >> 5);} 
     tobj2 = tobj2->next;}
    SDL_WarpMouse(SCREEN_X/2,SCREEN_Y/2);
            lives--;
         needStatusUpdate = true;
//            powerup = 0;
            secsleft = ticks + (1000 * level->timelimit);
            playing_fallsound = 0;
            if (lives < 0) { gamestate = 0; } else {
            ballpx = startx;
            ballpy = starty;
            ballpz = (float)(level->tile[(int)startx][(int)starty] >> 5)+1.0f;
            ballvx = ballvy = ballvz = ballax = ballay = ballaz = 0;
            createcloud(5,ballpx,ballpy,ballpz,0,0,-0.1,0,6,25,1,0,1);
            }
     }
     // put sane limitations on all velocities.
     if (ballvx > 50) ballvx = 50;
     if (ballvx < -50) ballvx = -50;
     if (ballvy > 50) ballvy = 50;
     if (ballvy < -50) ballvy = -50;
     if (ballvz > 50) ballvz = 50;
     if (ballvz < -50) ballvz = -50;
     
     // adjust ball.
     ballpx += ballvx*delta;
     ballpy += ballvy*delta;
     ballpz += ballvz*delta;
     
     ballrx += 100*ballvx*delta;
     ballry += 100*ballvy*delta;
     } else {
       if (secsleft <= ticks)
        {
         leveldone = 0;
         level = level->next;
         if (level == NULL) gamestate=0; else {
#ifdef THREADED
                   SDL_mutexP(lvlmutex);
#endif
                   destroy_level(); init_level();
#ifdef THREADED
                   SDL_mutexV(lvlmutex);
#endif
         }
       }
     }
    SDL_Delay (1);
#ifdef THREADED
   }
#endif
   return 0;
}
