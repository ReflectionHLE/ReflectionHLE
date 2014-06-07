/* Catacomb Abyss Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "ID_HEADS.H"
#include <MATH.H>
#include <VALUES.H>

//#define PROFILE

/*
=============================================================================

						 GLOBAL CONSTANTS

=============================================================================
*/

#define PI	3.141592657

#define ROTATE_SPEED		(6)

#define FL_QUICK 0x01

#define FL_CLEAR		0x01

#define GEM_SHIFT		1
#define FL_RGEM		0x02
#define FL_GGEM		0x04
#define FL_BGEM		0x08
#define FL_YGEM		0x10
#define FL_PGEM		0x20

#define MAXBOLTS		10
#define MAXNUKES		10
#define MAXPOTIONS	10

#define NUKE_COST		(1000)
#define BOLT_COST    (1200)
#define POTION_COST	(1300)

#define NUKE_COST_TXT		("1000")		// Allows for Q&D positioning..
#define BOLT_COST_TXT    	("1200")
#define POTION_COST_TXT		("1300")

#define RADARX	31															// bytes
#define RADARY	11                                           // pixels
#define RADAR_WIDTH     51                                  //   "
#define RADAR_HEIGHT		51                                  //   "
#define RADAR_XCENTER	((RADARX*8)+(RADAR_WIDTH/2)+3)      //   "
#define RADAR_YCENTER	((RADARY-8)+(RADAR_HEIGHT/2)+5)       //   "
#define MAX_RADAR_BLIPS	60


#define RADAR_RADIUS			17
#define RADAR_RADIUS_NSEW	15
#define RADAR_X_IRADIUS		(110/5)
#define RADAR_Y_IRADIUS		(110/7)
#define RADAR_ICON_CENTER	4						// Center offset into icon.

#define NAMESTART	180
#define REMOVED_DOOR_TILE	NAMESTART

#define NEXT_LEVEL_CODE		0xff
#define REMOVE_DOOR_CODE	0xfe
#define CANT_OPEN_CODE		0xfd
#define EXP_WALL_CODE		0xfc

#define WALL_SKELETON_CODE 6

#define UNMARKGRCHUNK(chunk)	(grneeded[chunk]&=~ca_levelbit)

#define MOUSEINT	0x33

#define EXPWALLSTART	8
#define NUMEXPWALLS		7
#define WALLEXP			15

#define NUMFLOORS		62+5

#define NUMLATCHPICS	 (FIRSTWALLPIC-FIRSTLATCHPIC)+5
#define NUMSCALEPICS	 (FIRSTWALLPIC-FIRSTSCALEPIC)+5
#define NUMSCALEWALLS (LASTWALLPIC-FIRSTWALLPIC)+5


#define FLASHCOLOR	12
#define FLASHTICS	4


#define NUMLEVELS	21

#define VIEWX		0		// corner of view window
#define VIEWY		0
#define VIEWWIDTH	(40*8)		// size of view window				// 33
#define VIEWHEIGHT	(15*8)												// 18
#define VIEWXH		(VIEWX+VIEWWIDTH-1)
#define VIEWYH		(VIEWY+VIEWHEIGHT-1)

#define CENTERX		(VIEWX+VIEWWIDTH/2-1)	// middle of view window
#define CENTERY		(VIEWY+VIEWHEIGHT/2-1)

#define GLOBAL1		(1l<<16)
#define TILEGLOBAL  GLOBAL1
#define TILESHIFT	16l

#define MINDIST		(2*GLOBAL1/5)
#define FOCALLENGTH	(TILEGLOBAL)	// in global coordinates

#define ANGLES		360		// must be divisable by 4

#define MAPSIZE		64		// maps are 64*64 max
#define MAXACTORS	100		// max number of tanks, etc / map

#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3

#define SIGN(x) ((x)>0?1:-1)
#define ABS(x) ((int)(x)>0?(x):-(x))
#define LABS(x) ((long)(x)>0?(x):-(x))

#define	MAXSCALE	(VIEWWIDTH/2)


#define MAXBODY			64
#define MAXSHOTPOWER	56

#define SCREEN1START	0
#define SCREEN2START	8320

//#define STATUallptr);
void	TraceRay (unsigned angle);
fixed	FixedByFrac (fixed a, fixed b);
void	TransformPoint (fixed gx, fixed gy, int *screenx, unsigned *screenheight);
fixed	TransformX (fixed gx, fixed gy);
int	FollowTrace (fixed tracex, fixed tracey, long deltax, long deltay, int max);
void	ForwardTrace (void);
int	FinishWall (void);
int	TurnClockwise (void);
int	TurnCounterClockwise (void);
void	FollowWall (void);

void	NewScene (void);
void	BuildTables (void);


/*
=============================================================================

						 C3_SCALE DEFINITIONS

=============================================================================
*/


#define COMPSCALECODESTART	(65*6)		// offset to start of code in comp scaler

typedef struct
{
	unsigned	codeofs[65];
	unsigned	start[65];
	unsigned	width[65];
	byte		code[];
}	t_compscale;

typedef struct
{
	unsigned	width;
	unsigned	codeofs[64];
}	t_compshape;


extern unsigned	scaleblockwidth,
		scaleblockheight,
		scalet pixel of wall (may not be visable)
  unsigned	height1,height2,color,walllength,side;
	long	planecoord;
} walltype;

typedef enum
  {nothing,playerobj,bonusobj,orcobj,batobj,skeletonobj,trollobj,demonobj,
  mageobj,pshotobj,bigpshotobj,mshotobj,inertobj,bounceobj,grelmobj,
  gateobj,zombieobj,spookobj,wetobj,expobj,eyeobj,eshotobj,wallskelobj,
  solidobj} classtype;

typedef enum {north,east,south,west,northeast,southeast,southwest,
		  northwest,nodir} dirtype;		// a catacombs 2 carryover


typedef struct	statestruct
{
	int		shapenum;
	int		tictime;
	void	(*think) ();
	struct	statestruct	*next;
} statetype;


typedef struct objstruct
{
  enum {no,noalways,yes,always}	active;
  int		ticcount;
  classtype	obclass;
  statetype	*state;

  boolean	shootable;
  boolean	tileobject;		// true if entirely inside one tile

  long		distance;
  dirtype	dir;
  fixed 	x,y;
  unsigned	tilex,tiley;
  int	 	viewx;
  unsigned	viewheight;

  int 		angle;
  int		hitpoints;
  long		speed;

  unsigned	size;			// global radius for hit rect calculation
  fixed		xl,xh,yl,yh;	// hit rectangle

  int		temp1,temp2;
  struct	objstruct	*next,*prev;
} objtype;


typedef	enum	{ex_stillplaying,ex_died,ex_warped,ex_resetgame
	,ex_loadedgame,ex_victorious,ex_abort} exittype;


typedef struct {
	char x,y;
	unsigned ondoor,underdoor;
} doorinfo;

typedef struct {
	char x,y;
	short angle;
//	doorinfo doors[MAX_DOOR_STORAGE];
} levelinfo;

typedef	struct
{
	int		difficulty;
	int		mapon;
	int		bolts,nukes,potions,keys[4],scrolls[8],gems[5];
	long	score;
	int		body,shotpower;
	levelinfo levels[NUMLEVELS];
} gametype;

/*
=============================================================================

						 C3_MAIN DEFINITIONS

=============================================================================
*/

extern char inlevel[][2];
extern	char		str[80],str2[20];
extern	unsigned	tedlevelnum;
extern	boolean		tedlevel;
extern	gametype	gamestate;
extern	exittype	playstate;
extern 	char SlowMode;
extern   unsigned Flags;


void NewGame (void);
boolean	SaveTheGame(int file);
boolean	LoadTheGame(int file);
void ResetGame(void);
void ShutdownId (void);
void InitGame (void);
void Quit (char *error);
void TEDDeath(void);
void DemoLoop (void);
void SetupScalePic (unsigned picnum);
void SetupScaleWall (unsigned picnum);
void SetupScaling (void);
void main (void);

/*
=============================================================================

						 C3_GAME DEFINITIONS

=============================================================================
*/

extern	unsigned	latchpics[NUMLATCHPICS];
extern	unsigned	tileoffsets[NUMTILE16];
extern	unsigned	textstarts[27];


#define	L_CHARS		0
#define L_NOSHOT	1
#define L_SHOTBAR	2
#define L_NOBODY	3
#define L_BODYBAR	4


void ScanInfoPlane (void);
void ScanText (void);
void SetupGameLevel (void);
void Victory (void);
void Died (void);
void NormalScreen (void);
void DrawPlayScreen (void);
void LoadLatchMem (void);
void FizzleFade (unsigned source, unsigned dest,
	unsigned width,unsigned height, boolean abortable);
void FizzleOut (int showlevel);
void FreeUpMemory (void);
void GameLoop (void);


/*
=============================================================================

						 C3_PLAY DEFINITIONS

=============================================================================
*/

#define BGF_NIGHT					0x01			// it is officially night
#define BGF_NOT_LIGHTNING		0x02			// lightning flash has ended

extern byte BGFLAGS;

extern unsigned *skycolor,*groundcolor;

extern	ControlInfo	c;
extern	boolean		running,slowturn;

extern	int			bordertime;

extern	byte		tilemap[MAPSIZE][MAPSIZE];
extern	objtype		*actorat[MAPSIZE][MAPSIZE];
extern	byte		spotvis[MAPSIZE][MAPSIZE];

extern	objtype 	objlist[MAXACTORS],*new,*obj,*player;

extern	unsigned	farmapylookup[MAPSIZE];
extern	byte		*nearmapylookup[MAPSIZE];
extern	byte		update[];

extern	boolean		godmode,singlestep;
extern	int			extravbls;

extern	int			mousexmove,mouseymove;
extern	int			pointcount,pointsleft;


void CenterWindow(word w,word h);
void DebugMemory (void);
void PicturePause (void);
int  DebugKeys (void);
void CheckKeys (void);
void InitObjList (void);
void GetNewObj (boolean usedummy);
void RemoveObj (objtype *gone);
void PollControlls (void);
void PlayLoop (void);
void InitBgChange(short stimer, unsigned *scolors, short gtimer, unsigned *gcolors, byte flag);
char GetKeyChoice(char *choices,boolean clear);


/*
=============================================================================

						 C3_STATE DEFINITIONS

=============================================================================
*/

void SpawnNewObj (unsigned x, unsigned y, statetype *state, unsigned size);
void SpawnNewObjFrac (long x, long y, statetype *state, unsigned size);
boolean CheckHandAttack (objtype *ob);
void T_DoDamage (objtype *ob);
boolean Walk (objtype *ob);
void ChaseThink (objtype *obj, boolean diagonal);
void MoveObj (objtype *ob, long move);
boolean Chase (objtype *ob, boolean diagonal);

extern	dirtype opposite[9];

/*
=============================================================================

						 C3_TRACE DEFINITIONS

=============================================================================
*/

int FollowTrace (fixed tracex, fixed tracey, long deltax, long deltay, int max);
int BackTrace (int finish);
void ForwardTrace (void);
int FinishWall (void);
void InsideCorner (void);
void OutsideCorner (void);
void FollowWalls (void);

extern	boolean	aborttrace;

/*
=============================================================================

						 C3_DRAW DEFINITIONS

=============================================================================
*/

#define MAXWALLS	50
#define DANGERHIGH	45

#define	MIDWALL		(MAXWALLS/2)

//==========================================================================

extern	tilept	tile,lasttile,focal,left,mid,right;

extern	globpt	edge,view;

extern	unsigned screenloc[3];
extern	unsigned freelatch;

extern	int screenpage;

extern	boolean		fizzlein;

extern	long lasttimecount;

extern	int firstangle,lastangle;

extern	fixed prestep;

extern	int traceclip,tracetop;

extern	fixed sintable[ANGLES+ANGLES/4],*costable;

extern	fixed	viewx,viewy,viewsin,viewcos;			// the focal point
extern	int	viewangle;

extern	fixed scale,scaleglobal;
extern	unsigned slideofs;

extern	int zbuffer[VIEWXH+1];

extern	walltype	walls[MAXWALLS],*leftwall,*rightwall;


extern	fixed	tileglobal;
extern	fixed	focallength;
extern	fixed	mindist;
extern	int		viewheight;
extern	fixed scale;

extern	int	walllight1[NUMFLOORS];
extern	int	walldark1[NUMFLOORS];
extern	int	walllight2[NUMFLOORS];
extern	int	walldark2[NUMFLOORS];

extern unsigned topcolor,bottomcolor;

//==========================================================================

void	DrawLine (int xl, int xh, int y,int color);
void	DrawWall (walltype *wallptr);
void	TraceRay (unsigned angle);
fixed	FixedByFrac (fixed a, fixed b);
void	TransformPoint (fixed gx, fixed gy, int *screenx, unsigned *screenheight);
fixed	TransformX (fixed gx, fixed gy);
int	FollowTrace (fixed tracex, fixed tracey, long deltax, long deltay, int max);
void	ForwardTrace (void);
int	FinishWall (void);
int	TurnClockwise (void);
int	TurnCounterClockwise (void);
void	FollowWall (void);

void	NewScene (void);
void	BuildTables (void);


/*
===============================tarted% " TimerDelay25
   TimerDelay10
  pcLastSample  	alNoCheck%  pcLengthLeft  	SDL_Delayp alSoundq  
SDL_ShutPCr  alFXReg
  	pcarrierss  	modifierst  SoundNumber
  alBlock
  SDL_StartALu vSDL_CleanALv >	LocalTime  
SDL_ShutALw pcSoundx  SDL_ALSoundServicey  TimerDivisor   
MusicGroup/      
AdLibSound byteSMMode'size_t
	ptrdiff_tSoundCommon
soundnames+ActiveTrackZlongwordSDMode#fpos_tPCSoundScanCoblockdest;

extern	byte	plotpix[8];
extern	byte	bitmasks1[8][8];
extern	byte	bitmasks2[8][8];


extern	t_compscale _seg *scaledirectory[MAXSCALE+1];
extern	t_compshape _seg *shapedirectory[NUMSCALEPICS];
extern	memptr			walldirectory[NUMSCALEWALLS];
extern	unsigned	shapesize[MAXSCALE+1];

void 		DeplanePic (int picnum);
void ScaleShape (int xcenter, t_compshape _seg *compshape, unsigned scale);
unsigned	BuildCompShape (t_compshape _seg **finalspot);


/*
=============================================================================

						 C3_ASM DEFINITIONS

=============================================================================
*/

extern	unsigned	wallheight	[VIEWWIDTH];
extern	unsigned	wallwidth	[VIEWWIDTH];
extern	unsigned	wallseg		[VIEWWIDTH];
extern	unsigned	wallofs		[VIEWWIDTH];
extern	unsigned	screenbyte	[VIEWWIDTH];
extern	unsigned	screenbit	[VIEWWIDTH];
extern	unsigned	bitmasks	[64];

extern	long		wallscalecall;

void	ScaleWalls (void);

/*
=============================================================================

						 C3_WIZ DEFINITIONS

=============================================================================
*/

#define MAXHANDHEIGHT	72

extern	statetype s_pshot1;
extern	statetype s_pshot2;

extern statetype s_pshot_exp1;
extern statetype s_pshot_exp2;
extern statetype s_pshot_exp3;

extern	long	lastnuke;
extern	int		handheight;
extern	int		boltsleft;
extern short RadarXY[][3];

void DrawText (boolean draw_text_whether_it_needs_it_or_not);
char DisplayMsg(char *text,char *choices);

extern short RotateAngle;
extern boolean FreezeTime;

//void FaceDir(short x, short y, boolean StopTime);
//short CalcAngle(short dx, short dy);

void FaceAngle(short DestAngle);
void RotateView();
void InitRotate(short DestAngle);
short FaceDoor(short x, short y);


/*
=============================================================================

						 C3_ACT1 DEFINITIONS

=============================================================================
*/

extern short zombie_base_delay;

extern	statetype s_trollouch;
extern	statetype s_trolldie1;


extern	statetype s_orcpause;

extern	statetype s_orc1;
extern	statetype s_orc2;
extern	statetype s_orc3;
extern	statetype s_orc4;

extern	statetype s_orcattack1;
extern	statetype s_orcattack2;
extern	statetype s_orcattack3;

extern	statetype s_orcouch;

extern	statetype s_orcdie1;
extern	statetype s_orcdie2;
extern	statetype s_orcdie3;


extern	statetype s_demonouch;
extern	statetype s_demondie1;

extern	statetype s_mageouch;
extern	statetype s_magedie1;

extern	statetype s_grelouch;
extern	statetype s_greldie1;

extern	statetype s_batdie1;

extern	statetype s_zombie_death1;
extern	statetype s_zombie_ouch;

extern statetype s_spook0_1;
extern statetype s_spook0_2;
extern statetype s_spook0_3;
extern statetype s_spook0_4;
extern statetype s_spook0;
extern statetype s_spook1;
extern statetype s_spook2;
extern statetype s_spook3;
extern statetype s_spook4;
extern statetype s_spook5;
extern statetype s_spook6;
extern statetype s_spook_pause;
extern statetype s_spook_attack1;
extern statetype s_spook_attack2;
extern statetype s_spook_attack3;
extern statetype s_spookouch;
extern statetype s_spookdie;
extern statetype s_spookdie1;
extern statetype s_spookdie2;
extern statetype s_spookdie3;
extern statetype s_spookdie4;
extern statetype s_spookdie5;
extern statetype s_spookdie6;
extern statetype s_spookdie7;
extern statetype s_spook_wait;


extern statetype s_skel_pause;
extern statetype s_skel_1;
extern statetype s_skel_2;
extern statetype s_skel_3;
extern statetype s_skel_4;
extern statetype s_skel_attack1;
extern statetype s_skel_attack2;
extern statetype s_skel_attack3;
extern statetype s_skel_ouch;
extern statetype s_skel_die1;
extern statetype s_skel_die2;
extern statetype s_skel_die3;

extern statetype s_wet_pause;

extern statetype s_wet_bubbles1;
extern statetype s_wet_bubbles2;
extern statetype s_wet_bubbles3;
extern statetype s_wet_bubbles4;

extern statetype s_wet_peek;

extern statetype s_wet_rise1;
extern statetype s_wet_rise2;
extern statetype s_wet_rise3;
extern statetype s_wet_rise4;
extern statetype s_wet_rise5;

extern statetype s_wet_sink1;
extern statetype s_wet_sink2;
extern statetype s_wet_sink3;

extern statetype s_wet_walk1;
extern statetype s_wet_walk2;
extern statetype s_wet_walk3;
extern statetype s_wet_walk4;

extern statetype s_wet_attack1;
extern statetype s_wet_attack2;
extern statetype s_wet_attack3;
extern statetype s_wet_attack4;

extern statetype s_wet_ouch;

extern statetype s_wet_die1;
extern statetype s_wet_die2;
extern statetype s_wet_die3;
extern statetype s_wet_die4;
extern statetype s_wet_die5;


extern statetype s_eye_pause;

extern statetype s_eye_1;
extern statetype s_eye_2;
extern statetype s_eye_3;

//statetype s_eye_attack1 = {SKELETON_ATTACK_1PIC,20,NULL,&s_eye_attack2};
//statetype s_eye_attack2 = {SKELETON_ATTACK_2PIC,10,T_DoDamage,&s_eye_attack3};
//statetype s_eye_attack3 = {SKELETON_ATTACK_3PIC,40,NULL,&s_eye_pause};

extern statetype s_eye_ouch;
extern statetype s_eye_ouch2;

extern statetype s_eye_die1;
extern statetype s_eye_die2;
extern statetype s_eye_die3;

extern	statetype s_mshot1;
extern	statetype s_mshot2;

extern	statetype s_obj_gate1;
extern	statetype s_obj_gate2;
extern	statetype s_obj_gate3;
extern	statetype s_obj_gate4;

