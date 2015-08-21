// WL_MAIN.C

#include <conio.h>
#include "WL_DEF.H"
#pragma hdrstop


/*
=============================================================================

						   WOLFENSTEIN 3-D

					  An Id Software production

						   by John Carmack

=============================================================================
*/

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/


#define FOCALLENGTH     (0x5700l)               // in global coordinates
#define VIEWGLOBAL      0x10000                 // globals visable flush to wall

#define VIEWWIDTH       256                     // size of view window
#define VIEWHEIGHT      144

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

char            str[80],str2[20];
// *** PRE-V1.4 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
unsigned				tedlevelnum;
#else
int				tedlevelnum;
#endif
boolean         tedlevel;
boolean         nospr;
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
boolean		BibleQuizDisabled, FloorsDisabled;
char		titletxt[] = "SUPER 3D NOAH'S ARK (v1.0)";
#endif
boolean         IsA386;
int                     dirangle[9] = {0,ANGLES/8,2*ANGLES/8,3*ANGLES/8,4*ANGLES/8,
	5*ANGLES/8,6*ANGLES/8,7*ANGLES/8,ANGLES};
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
int	quiznumber = 0;
#endif

//
// proejection variables
//
fixed           focallength;
unsigned        screenofs;
int             viewwidth;
int             viewheight;
int             centerx;
int             shootdelta;                     // pixels away from centerx a target can be
fixed           scale,maxslope;
long            heightnumerator;
int                     minheightdiv;


void            Quit (char *error);

		// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
boolean         startgame,loadedgame
#if (!defined GAMEVER_RESTORATION_ANY_APO_PRE14) && (!defined GAMEVER_RESTORATION_N3D_WIS10)
		,virtualreality
#endif
		;
int             mouseadjustment;

char	configname[13]="CONFIG.";


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


/*
====================
=
= ReadConfig
=
====================
*/

void ReadConfig(void)
{
	int                     file;
	SDMode          sd;
	SMMode          sm;
	SDSMode         sds;


	if ( (file = open(configname,O_BINARY | O_RDONLY)) != -1)
	{
	//
	// valid config file
	//
		read(file,Scores,sizeof(HighScore) * MaxScores);

		read(file,&sd,sizeof(sd));
		read(file,&sm,sizeof(sm));
		read(file,&sds,sizeof(sds));

		read(file,&mouseenabled,sizeof(mouseenabled));
		read(file,&joystickenabled,sizeof(joystickenabled));
		read(file,&joypadenabled,sizeof(joypadenabled));
		read(file,&joystickprogressive,sizeof(joystickprogressive));
		read(file,&joystickport,sizeof(joystickport));

		read(file,&dirscan,sizeof(dirscan));
		read(file,&buttonscan,sizeof(buttonscan));
		read(file,&buttonmouse,sizeof(buttonmouse));
		read(file,&buttonjoy,sizeof(buttonjoy));

		read(file,&viewsize,sizeof(viewsize));
		read(file,&mouseadjustment,sizeof(mouseadjustment));
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
		read(file,&quiznumber,sizeof(quiznumber));
#endif

		close(file);

		// *** PRE-V1.4 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
		if (sd == sdm_AdLib && (!AdLibPresent || !SoundBlasterPresent))
#else
		if (sd == sdm_AdLib && !AdLibPresent && !SoundBlasterPresent)
#endif
		{
			sd = sdm_PC;
			sd = smm_Off;
		}

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
		if (sds == sds_SoundBlaster && !SoundBlasterPresent)
#else
		if ((sds == sds_SoundBlaster && !SoundBlasterPresent) ||
			(sds == sds_SoundSource && !SoundSourcePresent))
#endif
			sds = sds_Off;

		if (!MousePresent)
			mouseenabled = false;
		if (!JoysPresent[joystickport])
			joystickenabled = false;

		MainMenu[6].active=1;
		MainItems.curpos=0;
	}
	else
	{
	//
	// no config file, so select by hardware
	//
		if (SoundBlasterPresent || AdLibPresent)
		{
			sd = sdm_AdLib;
			sm = smm_AdLib;
		}
		else
		{
			sd = sdm_PC;
			sm = smm_Off;
		}

		if (SoundBlasterPresent)
			sds = sds_SoundBlaster;
		// *** S3DNA RESTORATION ***
#ifndef GAMEVER_RESTORATION_N3D_WIS10
		else if (SoundSourcePresent)
			sds = sds_SoundSource;
#endif
		else
			sds = sds_Off;

		if (MousePresent)
			mouseenabled = true;

		joystickenabled = false;
		joypadenabled = false;
		joystickport = 0;
		joystickprogressive = false;

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
		viewsize = 18;
#else
		viewsize = 15;
#endif
		mouseadjustment=5;
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
		quiznumber=0;
#endif
	}

	SD_SetMusicMode (sm);
	SD_SetSoundMode (sd);
	SD_SetDigiDevice (sds);

}


/*
====================
=
= WriteConfig
=
====================
*/

void WriteConfig(void)
{
	int                     file;

	file = open(configname,O_CREAT | O_BINARY | O_WRONLY,
				S_IREAD | S_IWRITE | S_IFREG);

	if (file != -1)
	{
		write(file,Scores,sizeof(HighScore) * MaxScores);

		write(file,&SoundMode,sizeof(SoundMode));
		write(file,&MusicMode,sizeof(MusicMode));
		write(file,&DigiMode,sizeof(DigiMode));

		write(file,&mouseenabled,sizeof(mouseenabled));
		write(file,&joystickenabled,sizeof(joystickenabled));
		write(file,&joypadenabled,sizeof(joypadenabled));
		write(file,&joystickprogressive,sizeof(joystickprogressive));
		write(file,&joystickport,sizeof(joystickport));

		write(file,&dirscan,sizeof(dirscan));
		write(file,&buttonscan,sizeof(buttonscan));
		write(file,&buttonmouse,sizeof(buttonmouse));
		write(file,&buttonjoy,sizeof(buttonjoy));

		write(file,&viewsize,sizeof(viewsize));
		write(file,&mouseadjustment,sizeof(mouseadjustment));
		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
		write(file,&quiznumber,sizeof(quiznumber));
#endif

		close(file);
	}
}


//===========================================================================


/*
========================
=
= Patch386
=
= Patch ldiv to use 32 bit instructions
=
========================
*/

char    *JHParmStrings[] = {"no386",nil};
void Patch386 (void)
{
extern void far jabhack2(void);
extern int far  CheckIs386(void);

	int     i;

	for (i = 1;i < _argc;i++)
		if (US_CheckParm(_argv[i],JHParmStrings) == 0)
		{
			IsA386 = false;
			return;
		}

	if (CheckIs386())
	{
		IsA386 = true;
		jabhack2();
	}
	else
		IsA386 = false;
}

//===========================================================================

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame (int difficulty,int episode)
{
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	memset (&LevelRatios,0,sizeof(LRstruct)*30);
#endif
	memset (&gamestate,0,sizeof(gamestate));
	gamestate.difficulty = difficulty;
	gamestate.weapon = gamestate.bestweapon
		= gamestate.chosenweapon = wp_pistol;
	gamestate.health = 100;
	gamestate.ammo = STARTAMMO;
	gamestate.lives = 3;
	gamestate.nextextra = EXTRAPOINTS;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	gamestate.mapon=episode;
	gamestate.maxammo=99;
	gamestate.ammo2 = gamestate.ammo3 = 0;
	gamestate.weaponinv[0] = 0;
	gamestate.weaponinv[1] = 0;
	gamestate.weaponinv[3] = 0;
	gamestate.weaponinv[2] = 0;
	gamestate.fullmap = false;
#else
	gamestate.episode=episode;
#endif

	startgame = true;
}

//===========================================================================

void DiskFlopAnim(int x,int y)
{
 static char which=0;
 if (!x && !y)
   return;
 VWB_DrawPic(x,y,C_DISKLOADING1PIC+which);
 VW_UpdateScreen();
 // *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
 which=which+1;
 which=which&3;
#else
 which^=1;
#endif
}


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
// This isn't found in the v1.0 EXE
#ifndef GAMEVER_RESTORATION_WL1_APO10
long DoChecksum(byte far *source,unsigned size,long checksum)
{
 unsigned i;

 for (i=0;i<size-1;i++)
   checksum += source[i]^source[i+1];

 return checksum;
}
#endif


/*
==================
=
= SaveTheGame
=
==================
*/

boolean SaveTheGame(int file,int x,int y)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
	// Comment out anything to do with checksumming and free size verifications, plus a bit more

#ifndef GAMEVER_RESTORATION_WL1_APO10
	struct diskfree_t dfree;
	long avail,size,checksum;
#endif
	objtype *ob,nullobj;


#ifndef GAMEVER_RESTORATION_WL1_APO10
	if (_dos_getdiskfree(0,&dfree))
	  Quit("Error in _dos_getdiskfree call");

	avail = (long)dfree.avail_clusters *
			dfree.bytes_per_sector *
			dfree.sectors_per_cluster;

	size = 0;
	for (ob = player; ob ; ob=ob->next)
	  size += sizeof(*ob);
	size += sizeof(nullobj);

	size += sizeof(gamestate) +
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
			sizeof(LRstruct)*30 +
#else
			sizeof(LRstruct)*8 +
#endif
			sizeof(tilemap) +
			sizeof(actorat) +
			sizeof(laststatobj) +
			sizeof(statobjlist) +
			sizeof(doorposition) +
			sizeof(pwallstate) +
			sizeof(pwallx) +
			sizeof(pwally) +
			sizeof(pwalldir) +
			sizeof(pwallpos);

	if (avail < size)
	{
	 Message(STR_NOSPACE1"\n"
			 STR_NOSPACE2);
	 return false;
	}

	checksum = 0;
#endif // GAMEVER_RESTORATION_WL1_APO10


	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)&gamestate,sizeof(gamestate));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&gamestate,sizeof(gamestate),checksum);
#endif

	DiskFlopAnim(x,y);
	// *** SHAREWARE V1.0 APOGEE + SOD (DEMO) V1.0+V1.4 FORMGEN + S3DNA RESTORATION ***
	// LevelRatios should have 8 entries in these versions of SOD (like WL1/WL6) but don't write anything in Wolf3D v1.0
#ifndef GAMEVER_RESTORATION_WL1_APO10
#if (defined SPEAR) && (!defined GAMEVER_RESTORATION_ANY_PRE_GT)
//#ifdef SPEAR
	CA_FarWrite (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*20);
	checksum = DoChecksum((byte far *)&LevelRatios[0],sizeof(LRstruct)*20,checksum);
#elif (defined GAMEVER_RESTORATION_N3D_WIS10)
	CA_FarWrite (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*30);
	checksum = DoChecksum((byte far *)&LevelRatios[0],sizeof(LRstruct)*30,checksum);
#else
	CA_FarWrite (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*8);
	checksum = DoChecksum((byte far *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
#endif

	DiskFlopAnim(x,y);
#endif
	CA_FarWrite (file,(void far *)tilemap,sizeof(tilemap));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)tilemap,sizeof(tilemap),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)actorat,sizeof(actorat));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)actorat,sizeof(actorat),checksum);
#endif

#ifndef GAMEVER_RESTORATION_WL1_APO10
	CA_FarWrite (file,(void far *)areaconnect,sizeof(areaconnect));
	CA_FarWrite (file,(void far *)areabyplayer,sizeof(areabyplayer));
#endif

	for (ob = player ; ob ; ob=ob->next)
	{
	 DiskFlopAnim(x,y);
	 CA_FarWrite (file,(void far *)ob,sizeof(*ob));
	}
	nullobj.active = ac_badobject;          // end of file marker
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)&nullobj,sizeof(nullobj));



	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)&laststatobj,sizeof(laststatobj));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&laststatobj,sizeof(laststatobj),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)statobjlist,sizeof(statobjlist));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)statobjlist,sizeof(statobjlist),checksum);
#endif

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)doorposition,sizeof(doorposition));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)doorposition,sizeof(doorposition),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)doorobjlist,sizeof(doorobjlist));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)doorobjlist,sizeof(doorobjlist),checksum);
#endif

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)&pwallstate,sizeof(pwallstate));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwallstate,sizeof(pwallstate),checksum);
#endif
	CA_FarWrite (file,(void far *)&pwallx,sizeof(pwallx));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwallx,sizeof(pwallx),checksum);
#endif
	CA_FarWrite (file,(void far *)&pwally,sizeof(pwally));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwally,sizeof(pwally),checksum);
#endif
	CA_FarWrite (file,(void far *)&pwalldir,sizeof(pwalldir));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwalldir,sizeof(pwalldir),checksum);
#endif
	CA_FarWrite (file,(void far *)&pwallpos,sizeof(pwallpos));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwallpos,sizeof(pwallpos),checksum);
#endif

#ifndef GAMEVER_RESTORATION_WL1_APO10
	//
	// WRITE OUT CHECKSUM
	//
	CA_FarWrite (file,(void far *)&checksum,sizeof(checksum));
#endif

	return(true);
}

//===========================================================================

/*
==================
=
= LoadTheGame
=
==================
*/

boolean LoadTheGame(int file,int x,int y)
{
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
	// Comment out anything to do with checksumming, plus a bit more

#ifndef GAMEVER_RESTORATION_WL1_APO10
	long checksum,oldchecksum;
#endif
	objtype *ob,nullobj;


#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = 0;
#endif

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&gamestate,sizeof(gamestate));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&gamestate,sizeof(gamestate),checksum);
#endif

	DiskFlopAnim(x,y);
	// *** SHAREWARE V1.0 APOGEE + SOD (DEMO) V1.0+V1.4 FORMGEN + S3DNA RESTORATION ***
	// LevelRatios should have 8 entries in these versions of SOD (like WL1/WL6) but don't read anything in Wolf3D v1.0
#ifndef GAMEVER_RESTORATION_WL1_APO10
#if (defined SPEAR) && (!defined GAMEVER_RESTORATION_ANY_PRE_GT)
//#ifdef SPEAR
	CA_FarRead (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*20);
	checksum = DoChecksum((byte far *)&LevelRatios[0],sizeof(LRstruct)*20,checksum);
#elif (defined GAMEVER_RESTORATION_N3D_WIS10)
	CA_FarRead (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*30);
	checksum = DoChecksum((byte far *)&LevelRatios[0],sizeof(LRstruct)*30,checksum);
#else
	CA_FarRead (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*8);
	checksum = DoChecksum((byte far *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
#endif

	DiskFlopAnim(x,y);
#endif
	SetupGameLevel ();

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)tilemap,sizeof(tilemap));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)tilemap,sizeof(tilemap),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)actorat,sizeof(actorat));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)actorat,sizeof(actorat),checksum);
#endif

#ifndef GAMEVER_RESTORATION_WL1_APO10
	CA_FarRead (file,(void far *)areaconnect,sizeof(areaconnect));
	CA_FarRead (file,(void far *)areabyplayer,sizeof(areabyplayer));
#endif



	InitActorList ();
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)player,sizeof(*player));

	while (1)
	{
	 DiskFlopAnim(x,y);
		CA_FarRead (file,(void far *)&nullobj,sizeof(nullobj));
		if (nullobj.active == ac_badobject)
			break;
		GetNewActor ();
		// *** SHAREWARE V1.0+1.1 APOGEE RESTORATION ***
#if (defined GAMEVER_RESTORATION_WL1_APO10) || (defined GAMEVER_RESTORATION_WL1_APO11)
		memcpy (new,&nullobj,sizeof(nullobj));
#else
	 // don't copy over the links
		memcpy (new,&nullobj,sizeof(nullobj)-4);
#endif
	}



	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&laststatobj,sizeof(laststatobj));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&laststatobj,sizeof(laststatobj),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)statobjlist,sizeof(statobjlist));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)statobjlist,sizeof(statobjlist),checksum);
#endif

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)doorposition,sizeof(doorposition));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)doorposition,sizeof(doorposition),checksum);
#endif
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)doorobjlist,sizeof(doorobjlist));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)doorobjlist,sizeof(doorobjlist),checksum);
#endif

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&pwallstate,sizeof(pwallstate));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwallstate,sizeof(pwallstate),checksum);
#endif
	CA_FarRead (file,(void far *)&pwallx,sizeof(pwallx));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwallx,sizeof(pwallx),checksum);
#endif
	CA_FarRead (file,(void far *)&pwally,sizeof(pwally));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwally,sizeof(pwally),checksum);
#endif
	CA_FarRead (file,(void far *)&pwalldir,sizeof(pwalldir));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwalldir,sizeof(pwalldir),checksum);
#endif
	CA_FarRead (file,(void far *)&pwallpos,sizeof(pwallpos));
#ifndef GAMEVER_RESTORATION_WL1_APO10
	checksum = DoChecksum((byte far *)&pwallpos,sizeof(pwallpos),checksum);
#endif

#ifndef GAMEVER_RESTORATION_WL1_APO10
	CA_FarRead (file,(void far *)&oldchecksum,sizeof(oldchecksum));

	if (oldchecksum != checksum)
	{
	 Message(STR_SAVECHT1"\n"
			 STR_SAVECHT2"\n"
			 STR_SAVECHT3"\n"
			 STR_SAVECHT4);

	 IN_ClearKeysDown();
	 IN_Ack();

	 gamestate.score = 0;
	 gamestate.lives = 1;
	 gamestate.weapon =
	   gamestate.chosenweapon =
	   gamestate.bestweapon = wp_pistol;
	 gamestate.ammo = 8;
	 // *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	 gamestate.ammo2 = gamestate.ammo3 = 0;
	 gamestate.weaponinv[0] = 0;
	 gamestate.weaponinv[1] = 0;
	 gamestate.weaponinv[3] = 0;
	 gamestate.weaponinv[2] = 0;
	 gamestate.fullmap = false;
#endif
	}
#endif

	return true;
}

//===========================================================================

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId (void)
{
	US_Shutdown ();
	SD_Shutdown ();
	PM_Shutdown ();
	IN_Shutdown ();
	VW_Shutdown ();
	CA_Shutdown ();
	MM_Shutdown ();
}


//===========================================================================

/*
==================
=
= BuildTables
=
= Calculates:
=
= scale                 projection constant
= sintable/costable     overlapping fractional tables
=
==================
*/

const   float   radtoint = (float)FINEANGLES/2/PI;

void BuildTables (void)
{
  int           i;
  float         angle,anglestep;
  double        tang;
  fixed         value;


//
// calculate fine tangents
//

	for (i=0;i<FINEANGLES/8;i++)
	{
		tang = tan( (i+0.5)/radtoint);
		finetangent[i] = tang*TILEGLOBAL;
		finetangent[FINEANGLES/4-1-i] = 1/tang*TILEGLOBAL;
	}

//
// costable overlays sintable with a quarter phase shift
// ANGLES is assumed to be divisable by four
//
// The low word of the value is the fraction, the high bit is the sign bit,
// bits 16-30 should be 0
//

  angle = 0;
  anglestep = PI/2/ANGLEQUAD;
  for (i=0;i<=ANGLEQUAD;i++)
  {
	value=GLOBAL1*sin(angle);
	sintable[i]=
	  sintable[i+ANGLES]=
	  sintable[ANGLES/2-i] = value;
	sintable[ANGLES-i]=
	  sintable[ANGLES/2+i] = value | 0x80000000l;
	angle += anglestep;
  }

}

//===========================================================================


/*
====================
=
= CalcProjection
=
= Uses focallength
=
====================
*/

void CalcProjection (long focal)
{
	int             i;
	long            intang;
	float   angle;
	double  tang;
	double  planedist;
	double  globinhalf;
	int             halfview;
	double  halfangle,facedist;


	focallength = focal;
	facedist = focal+MINDIST;
	halfview = viewwidth/2;                                 // half view in pixels

//
// calculate scale value for vertical height calculations
// and sprite x calculations
//
	scale = halfview*facedist/(VIEWGLOBAL/2);

//
// divide heightnumerator by a posts distance to get the posts height for
// the heightbuffer.  The pixel height is height>>2
//
	heightnumerator = (TILEGLOBAL*scale)>>6;
	minheightdiv = heightnumerator/0x7fff +1;

//
// calculate the angle offset from view angle of each pixel's ray
//

	for (i=0;i<halfview;i++)
	{
	// start 1/2 pixel over, so viewangle bisects two middle pixels
		tang = (long)i*VIEWGLOBAL/viewwidth/facedist;
		angle = atan(tang);
		intang = angle*radtoint;
		pixelangle[halfview-1-i] = intang;
		pixelangle[halfview+i] = -intang;
	}

//
// if a point's abs(y/x) is greater than maxslope, the point is outside
// the view area
//
	maxslope = finetangent[pixelangle[0]];
	maxslope >>= 8;
}



//===========================================================================

/*
===================
=
= SetupWalls
=
= Map tile values to scaled pics
=
===================
*/

void SetupWalls (void)
{
	int     i;

	for (i=1;i<MAXWALLTILES;i++)
	{
		horizwall[i]=(i-1)*2;
		vertwall[i]=(i-1)*2+1;
	}
}

//===========================================================================

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_RESTORATION_N3D_WIS10
/*
==========================
=
= SignonScreen
=
==========================
*/

void SignonScreen (void)                        // VGA version
{
	unsigned        segstart,seglength;

	VL_SetVGAPlaneMode ();
	VL_TestPaletteSet ();
	VL_SetPalette (&gamepal);

	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_ANY_APO_PRE14
	if (!virtualreality)
#endif
	{
		VW_SetScreen(0x8000,0);
		VL_MungePic (&introscn,320,200);
		VL_MemToScreen (&introscn,320,200,0,0);
		VW_SetScreen(0,0);
	}

//
// reclaim the memory from the linked in signon screen
//
	segstart = FP_SEG(&introscn);
	seglength = 64000/16;
	if (FP_OFF(&introscn))
	{
		segstart++;
		seglength--;
	}
	MML_UseSpace (segstart,seglength);
}


/*
==========================
=
= FinishSignon
=
==========================
*/

void FinishSignon (void)
{

#ifndef SPEAR
	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
	VW_Bar (0,185,300,15,peekb(0xa000,0));
#else
	VW_Bar (0,189,300,11,peekb(0xa000,0));
#endif
	WindowX = 0;
	WindowW = 320;
	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
	PrintY = 188;
#else
	PrintY = 190;
#endif

	#ifndef JAPAN
	SETFONTCOLOR(14,4);

	#ifdef SPANISH
	US_CPrint ("Oprima una tecla");
	#else
	US_CPrint ("Press a key");
	#endif

	#endif

	if (!NoWait)
		IN_Ack ();

	#ifndef JAPAN
	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
	VW_Bar (0,185,320,15,peekb(0xa000,0));

	PrintY = 188;
#else
	VW_Bar (0,189,300,11,peekb(0xa000,0));

	PrintY = 190;
#endif
	SETFONTCOLOR(10,4);

	#ifdef SPANISH
	US_CPrint ("pensando...");
	#else
	US_CPrint ("Working...");
	#endif

	#endif

	SETFONTCOLOR(0,15);
#else
	if (!NoWait)
		VW_WaitVBL(3*70);
#endif
}
#endif // S3DNA RESTORATION

//===========================================================================

/*
=================
=
= MS_CheckParm
=
=================
*/

boolean MS_CheckParm (char far *check)
{
	int             i;
	char    *parm;

	for (i = 1;i<_argc;i++)
	{
		parm = _argv[i];

		while ( !isalpha(*parm) )       // skip - / \ etc.. in front of parm
			if (!*parm++)
				break;                          // hit end of string without an alphanum

		if ( !_fstricmp(check,parm) )
			return true;
	}

	return false;
}

//===========================================================================

/*
=====================
=
= InitDigiMap
=
=====================
*/

static  int     wolfdigimap[] =
		{
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
			SCHUTZADSND,            0,
			DIESND,                 1,
			GETITEMSND,             2,
			GUTENTAGSND,            3,
			CLOSEDOORSND,           4,
			MISSILEHITSND,          5,
			MISSILEFIRESND,         6,
			DOGATTACKSND,           7,
			GULPSND,                8,
			SLEEPSND,               9,
			ERLAUBENSND,            10,
			BONUS1UPSND,            11,
			ATKGATLINGSND,          12,
			KEINSND,                13,
			DOGBARKSND,             14,
			ATKPISTOLSND,           15,
			ENDBONUS1SND,           16,
			EINESND,                17,
			GETKEYSND,              18,
			ATKKNIFESND,            19,
			SCHABBSHASND,           20,
			MECHSTEPSND,            21,
			OPENDOORSND,            22,
			SPIONSND,               23,
			GROWLSND,               24,
			TAKEDAMAGESND,          25,
			PLAYERDEATHSND,         26,

			PUSHWALLSND,            27,
			HALTSND,                28,
			ATKMACHINEGUNSND,       29,
			NAZIFIRESND,            30,
			WATERMELONHITSND,       31,
			WATERMELONFIRESND,      32,
#elif (!defined SPEAR) // *** S3DNA RESTORATION ***
			// These first sounds are in the upload version
//#ifndef SPEAR
			HALTSND,                0,
			DOGBARKSND,             1,
			CLOSEDOORSND,           2,
			OPENDOORSND,            3,
			ATKMACHINEGUNSND,       4,
			ATKPISTOLSND,           5,
			ATKGATLINGSND,          6,
			SCHUTZADSND,            7,
			GUTENTAGSND,            8,
			MUTTISND,               9,
			BOSSFIRESND,            10,
			SSFIRESND,              11,
			DEATHSCREAM1SND,        12,
			DEATHSCREAM2SND,        13,
			DEATHSCREAM3SND,        13,
			TAKEDAMAGESND,          14,
			PUSHWALLSND,            15,

			LEBENSND,               20,
			NAZIFIRESND,            21,
			SLURPIESND,             22,

			YEAHSND,				32,

#ifndef UPLOAD
			// These are in all other episodes
			DOGDEATHSND,            16,
			AHHHGSND,               17,
			DIESND,                 18,
			EVASND,                 19,

			TOT_HUNDSND,            23,
			MEINGOTTSND,            24,
			SCHABBSHASND,           25,
			HITLERHASND,            26,
			SPIONSND,               27,
			NEINSOVASSND,           28,
			DOGATTACKSND,           29,
			LEVELDONESND,           30,
			MECHSTEPSND,			31,

			SCHEISTSND,				33,
			DEATHSCREAM4SND,		34,		// AIIEEE
			DEATHSCREAM5SND,		35,		// DEE-DEE
			DONNERSND,				36,		// EPISODE 4 BOSS DIE
			EINESND,				37,		// EPISODE 4 BOSS SIGHTING
			ERLAUBENSND,			38,		// EPISODE 6 BOSS SIGHTING
			DEATHSCREAM6SND,		39,		// FART
			DEATHSCREAM7SND,		40,		// GASP
			DEATHSCREAM8SND,		41,		// GUH-BOY!
			DEATHSCREAM9SND,		42,		// AH GEEZ!
			KEINSND,				43,		// EPISODE 5 BOSS SIGHTING
			MEINSND,				44,		// EPISODE 6 BOSS DIE
			ROSESND,				45,		// EPISODE 5 BOSS DIE

#endif
#else
//
// SPEAR OF DESTINY DIGISOUNDS
//
			HALTSND,                0,
			CLOSEDOORSND,           2,
			OPENDOORSND,            3,
			ATKMACHINEGUNSND,       4,
			ATKPISTOLSND,           5,
			ATKGATLINGSND,          6,
			SCHUTZADSND,            7,
			BOSSFIRESND,            8,
			SSFIRESND,              9,
			DEATHSCREAM1SND,        10,
			DEATHSCREAM2SND,        11,
			TAKEDAMAGESND,          12,
			PUSHWALLSND,            13,
			AHHHGSND,               15,
			LEBENSND,               16,
			NAZIFIRESND,            17,
			SLURPIESND,             18,
			LEVELDONESND,           22,
			DEATHSCREAM4SND,		23,		// AIIEEE
			DEATHSCREAM3SND,        23,		// DOUBLY-MAPPED!!!
			DEATHSCREAM5SND,		24,		// DEE-DEE
			DEATHSCREAM6SND,		25,		// FART
			DEATHSCREAM7SND,		26,		// GASP
			DEATHSCREAM8SND,		27,		// GUH-BOY!
			DEATHSCREAM9SND,		28,		// AH GEEZ!
			GETGATLINGSND,			38,		// Got Gat replacement

#ifndef SPEARDEMO
			DOGBARKSND,             1,
			DOGDEATHSND,            14,
			SPIONSND,               19,
			NEINSOVASSND,           20,
			DOGATTACKSND,           21,
			TRANSSIGHTSND,			29,		// Trans Sight
			TRANSDEATHSND,			30,		// Trans Death
			WILHELMSIGHTSND,		31,		// Wilhelm Sight
			WILHELMDEATHSND,		32,		// Wilhelm Death
			UBERDEATHSND,			33,		// Uber Death
			KNIGHTSIGHTSND,			34,		// Death Knight Sight
			KNIGHTDEATHSND,			35,		// Death Knight Death
			ANGELSIGHTSND,			36,		// Angel Sight
			ANGELDEATHSND,			37,		// Angel Death
			GETSPEARSND,			39,		// Got Spear replacement
#endif
#endif
			LASTSOUND
		};


void InitDigiMap (void)
{
	int                     *map;

	for (map = wolfdigimap;*map != LASTSOUND;map += 2)
		DigiMap[map[0]] = map[1];


}


// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_WL1_APO10
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
CP_iteminfo	MusicItems={CTL_X,40,11,0,32};
CP_itemtype far MusicMenu[]=
	{
		{1,"Song 1",0},
		{1,"It's All Good!",0},
		{1,"Song 3",0},
		{1,"Song 4",0},
		{1,"Feed-time Shuffle",0},
		{1,"Song 6",0},
		{1,"Song 7",0},
		{1,"Song 8",0},
		{1,"Song 9",0},
		{1,"The Happy Song",0},
		{1,"Song 11",0},
	};
#elif (!defined SPEAR)
//#ifndef SPEAR
CP_iteminfo	MusicItems={CTL_X,CTL_Y,6,0,32};
CP_itemtype far MusicMenu[]=
	{
		{1,"Get Them!",0},
		{1,"Searching",0},
		{1,"P.O.W.",0},
		{1,"Suspense",0},
		{1,"War March",0},
		{1,"Around The Corner!",0},

		{1,"Nazi Anthem",0},
		{1,"Lurking...",0},
		{1,"Going After Hitler",0},
		{1,"Pounding Headache",0},
		{1,"Into the Dungeons",0},
		{1,"Ultimate Conquest",0},

		{1,"Kill the S.O.B.",0},
		{1,"The Nazi Rap",0},
		{1,"Twelfth Hour",0},
		{1,"Zero Hour",0},
		{1,"Ultimate Conquest",0},
		{1,"Wolfpack",0}
	};
#else
CP_iteminfo MusicItems={CTL_X,CTL_Y-20,9,0,32};
CP_itemtype far MusicMenu[]=
   {
		{1,"Funky Colonel Bill",0},
		{1,"Death To The Nazis",0},
		{1,"Tiptoeing Around",0},
		{1,"Is This THE END?",0},
		{1,"Evil Incarnate",0},
		{1,"Jazzin' Them Nazis",0},
		{1,"Puttin' It To The Enemy",0},
		{1,"The SS Gonna Get You",0},
		{1,"Towering Above",0}
	};
#endif

#ifndef SPEARDEMO
void DoJukebox(void)
{
	int which,lastsong=-1;
	unsigned start,songs[]=
		{
// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
			SONG1_MUS,
			ALL_GOOD_MUS,
			SONG3_MUS,
			SONG4_MUS,
			FEEDTIME_MUS,
			SONG6_MUS,
			SONG7_MUS,
			SONG8_MUS,
			SONG9_MUS,
			THEHAPPY_MUS,
			SONG11_MUS,
#elif (!defined SPEAR)
//#ifndef SPEAR
			GETTHEM_MUS,
			SEARCHN_MUS,
			POW_MUS,
			SUSPENSE_MUS,
			WARMARCH_MUS,
			CORNER_MUS,

			NAZI_OMI_MUS,
			PREGNANT_MUS,
			GOINGAFT_MUS,
			HEADACHE_MUS,
			DUNGEON_MUS,
			ULTIMATE_MUS,

			INTROCW3_MUS,
			NAZI_RAP_MUS,
			TWELFTH_MUS,
			ZEROHOUR_MUS,
			ULTIMATE_MUS,
			PACMAN_MUS
#else
			XFUNKIE_MUS,             // 0
			XDEATH_MUS,              // 2
			XTIPTOE_MUS,             // 4
			XTHEEND_MUS,             // 7
			XEVIL_MUS,               // 17
			XJAZNAZI_MUS,            // 18
			XPUTIT_MUS,              // 21
			XGETYOU_MUS,             // 22
			XTOWER2_MUS              // 23
#endif
		};
	struct dostime_t time;



	IN_ClearKeysDown();
	if (!AdLibPresent && !SoundBlasterPresent)
		return;


	MenuFadeOut();

#ifndef SPEAR
// *** S3DNA RESTORATION ***
#if (!defined UPLOAD) && (!defined GAMEVER_RESTORATION_N3D_WIS10)
//#ifndef UPLOAD
	_dos_gettime(&time);
	start = (time.hsecond%3)*6;
#else
	start = 0;
#endif
#else
	start = 0;
#endif


	CA_CacheGrChunk (STARTFONT+1);
#ifdef SPEAR
	CacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
#else
	CacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
#endif
	CA_LoadAllSounds ();

	fontnumber=1;
	ClearMScreen ();
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	VWB_DrawPic(112,192,C_MOUSELBACKPIC);
#else
	VWB_DrawPic(112,184,C_MOUSELBACKPIC);
	DrawStripes (10);
#endif
	SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	DrawWindow (CTL_X-2,CTL_Y-36,280,13*12,BKGDCOLOR);
#elif (!defined SPEAR)
//#ifndef SPEAR
	DrawWindow (CTL_X-2,CTL_Y-6,280,13*7,BKGDCOLOR);
#else
	DrawWindow (CTL_X-2,CTL_Y-26,280,13*10,BKGDCOLOR);
#endif

	DrawMenu (&MusicItems,&MusicMenu[start]);

	SETFONTCOLOR (READHCOLOR,BKGDCOLOR);
	PrintY=15;
	WindowX = 0;
	WindowY = 320;
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	US_CPrint ("Noah Tunes");
#else
	US_CPrint ("Robert's Jukebox");
#endif

	SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);
	VW_UpdateScreen();
	MenuFadeIn();

	do
	{
		which = HandleMenu(&MusicItems,&MusicMenu[start],NULL);
		if (which>=0)
		{
			if (lastsong >= 0)
				MusicMenu[start+lastsong].active = 1;

			StartCPMusic(songs[start + which]);
			MusicMenu[start+which].active = 2;
			DrawMenu (&MusicItems,&MusicMenu[start]);
			VW_UpdateScreen();
			lastsong = which;
		}
	} while(which>=0);

	MenuFadeOut();
	IN_ClearKeysDown();
	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_ANY_APO_PRE14
#ifdef SPEAR
	UnCacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
#else
	UnCacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
#endif
#endif
}
#endif
#endif // GAMEVER_RESTORATION_WL1_APO10


/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

void InitGame (void)
{
	int                     i,x,y;
	unsigned        *blockstart;

	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	BibleQuizDisabled = false;
	if (MS_CheckParm ("noquestions"))
		BibleQuizDisabled = true;
	FloorsDisabled = false;
	if (MS_CheckParm ("nofloors"))
		FloorsDisabled = true;
#elif (!defined GAMEVER_RESTORATION_ANY_APO_PRE14)
	if (MS_CheckParm ("virtual"))
		virtualreality = true;
	else
		virtualreality = false;
#endif

	MM_Startup ();                  // so the signon screen can be freed

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_RESTORATION_N3D_WIS10
	SignonScreen ();
#endif

	VW_Startup ();
	IN_Startup ();
	PM_Startup ();
	PM_UnlockMainMem ();
	SD_Startup ();
	CA_Startup ();
	US_Startup ();


// *** SHAREWARE V1.0 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_WL1_APO10
	if (mminfo.mainmem < 240000L)
#elif (defined GAMEVER_RESTORATION_N3D_WIS10)
	if (mminfo.mainmem < 275000L && !MS_CheckParm(GAMEVER_RESTORATION_W3D_DEBUGPARM))
#elif (!defined SPEAR)
//#ifndef SPEAR
	if (mminfo.mainmem < 235000L)
#else
	if (mminfo.mainmem < 257000L && !MS_CheckParm("debugmode"))
#endif
	{
		memptr screen;

		CA_CacheGrChunk (ERRORSCREEN);
		screen = grsegs[ERRORSCREEN];
		ShutdownId();
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_WL1_APO10
		movedata ((unsigned)screen,7+8*160,0xb800,0,15*160);
#else
		movedata ((unsigned)screen,7+7*160,0xb800,0,17*160);
#endif
		gotoxy (1,23);
		exit(1);
	}


//
// build some tables
//
	InitDigiMap ();

	for (i=0;i<MAPSIZE;i++)
	{
		nearmapylookup[i] = &tilemap[0][0]+MAPSIZE*i;
		farmapylookup[i] = i*64;
	}

	for (i=0;i<PORTTILESHIGH;i++)
		uwidthtable[i] = UPDATEWIDE*i;

	blockstart = &blockstarts[0];
	for (y=0;y<UPDATEHIGH;y++)
		for (x=0;x<UPDATEWIDE;x++)
			*blockstart++ = SCREENWIDTH*16*y+x*TILEWIDTH;

	updateptr = &update[0];
	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_WL1_APO10
	*(unsigned *)(updateptr+UPDATEWIDE*UPDATEHIGH) = UPDATETERMINATE;
#endif

	bufferofs = 0;
	displayofs = 0;
	ReadConfig ();


	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_RESTORATION_N3D_WIS10
//
// HOLDING DOWN 'M' KEY?
//

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#if (!defined SPEARDEMO) && (!defined GAMEVER_RESTORATION_WL1_APO10)
//#ifndef SPEARDEMO
	if (Keyboard[sc_M])
	  DoJukebox();
	else
#endif
//
// draw intro screen stuff
//
	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_ANY_APO_PRE14
	if (!virtualreality)
#endif
		IntroScreen ();
#endif // S3DNA RESTORATION

//
// load in and lock down some basic chunks
//

	CA_CacheGrChunk(STARTFONT);
	MM_SetLock (&grsegs[STARTFONT],true);

	// *** S3DNA RESTORATION ***
#ifndef GAMEVER_RESTORATION_N3D_WIS10
	LoadLatchMem ();
#endif
	BuildTables ();          // trig tables
	SetupWalls ();

#if 0
{
int temp,i;
temp = viewsize;
	profilehandle = open("SCALERS.TXT", O_CREAT | O_WRONLY | O_TEXT);
for (i=1;i<20;i++)
	NewViewSize(i);
viewsize = temp;
close(profilehandle);
}
#endif

	NewViewSize (viewsize);


//
// initialize variables
//
	InitRedShifts ();
	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#ifndef GAMEVER_RESTORATION_N3D_WIS10
#ifndef GAMEVER_RESTORATION_ANY_APO_PRE14
	if (!virtualreality)
#endif
		FinishSignon();
#endif

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_WL1_APO10
	displayofs = PAGE1START;
	bufferofs = PAGE2START;
#endif

	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	if (MS_CheckParm ("CHECK"))
	{
		printf("Press any key...");
		IN_Ack();
	}
	else
		IN_UserInput (TickBase);

	VL_SetVGAPlaneMode ();
	VL_TestPaletteSet ();
	VL_SetPalette (&gamepal);
	VW_FadeOut ();

	LoadLatchMem ();
#elif (defined GAMEVER_RESTORATION_ANY_APO_PRE14)
	if (virtualreality)
	{
		NoWait = true;
		geninterrupt(0x60);
	}
#endif
}

//===========================================================================

/*
==========================
=
= SetViewSize
=
==========================
*/

boolean SetViewSize (unsigned width, unsigned height)
{
	viewwidth = width&~15;                  // must be divisable by 16
	viewheight = height&~1;                 // must be even
	centerx = viewwidth/2-1;
	shootdelta = viewwidth/10;
	screenofs = ((200-STATUSLINES-viewheight)/2*SCREENWIDTH+(320-viewwidth)/8);

//
// calculate trace angles and projection constants
//
	CalcProjection (FOCALLENGTH);

//
// build all needed compiled scalers
//
//	MM_BombOnError (false);
	SetupScaling (viewwidth*1.5);
#if 0
	MM_BombOnError (true);
	if (mmerror)
	{
		Quit ("Can't build scalers!");
		mmerror = false;
		return false;
	}
#endif
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	SetPlaneViewSize ();
#endif
	return true;
}


void ShowViewSize (int width)
{
	int     oldwidth,oldheight;

	oldwidth = viewwidth;
	oldheight = viewheight;

	viewwidth = width*16;
	viewheight = width*16*HEIGHTRATIO;
	DrawPlayBorder ();

	viewheight = oldheight;
	viewwidth = oldwidth;
}


void NewViewSize (int width)
{
	CA_UpLevel ();
	MM_SortMem ();
	viewsize = width;
	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
resize:
	if (!SetViewSize (width*16,width*16*HEIGHTRATIO))
	{
		--width;
		goto resize;
	}
#else
	SetViewSize (width*16,width*16*HEIGHTRATIO);
#endif
	CA_DownLevel ();
}



//===========================================================================

/*
==========================
=
= Quit
=
==========================
*/

void Quit (char *error)
{
	unsigned        finscreen;
	memptr	screen;

	// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#if (!defined GAMEVER_RESTORATION_ANY_APO_PRE14) && (!defined GAMEVER_RESTORATION_N3D_WIS10)
	if (virtualreality)
		geninterrupt(0x61);
#endif

	// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_WL1_APO10
	ClearMemory ();
#endif
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
 if (CA_Started)
 {
#endif
	if (!*error)
	{
	 #ifndef JAPAN
	 CA_CacheGrChunk (ORDERSCREEN);
	 screen = grsegs[ORDERSCREEN];
	 #endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_ANY_APO_PRE14
	 WriteConfig ();
#endif
	}
	else
	{
	 CA_CacheGrChunk (ERRORSCREEN);
	 screen = grsegs[ERRORSCREEN];
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
 }
#endif
	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
	WriteConfig ();
#endif
	ShutdownId ();

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
 if (CA_Started)
 {
	clrscr();
#endif
	if (error && *error)
	{
	  // *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	  movedata ((unsigned)screen,0,0xb800,0,7*160);
#else
	  movedata ((unsigned)screen,7,0xb800,0,7*160);
#endif
	  gotoxy (10,4);
	  puts(error);
	  gotoxy (1,8);
	  exit(1);
	}
	// *** PRE-V1.4 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_ANY_APO_PRE14
	else
#endif
	if (!error || !(*error))
	{
		clrscr();
		// *** ACTIVISION RELEASES RESTORATION ***
		// This should be commented out in the 1.4 Activision releases
		// of Wolf3D and SOD (no copy protection), but compiled in the
		// (shareware/registered) Apogee and SOD (demo) releases,
		// as well as the other "GOODTIMES" releases
		#if (!defined JAPAN) && (!defined GAMEVER_RESTORATION_ANY_ACT14)
		//#ifndef JAPAN
		// *** S3DNA RESTORATION ***
		#ifdef GAMEVER_RESTORATION_N3D_WIS10
		movedata ((unsigned)screen,0,0xb800,0,4000);
		#else
		movedata ((unsigned)screen,7,0xb800,0,4000);
		#endif
		gotoxy(1,24);
		#endif
//asm	mov	bh,0
//asm	mov	dh,23	// row
//asm	mov	dl,0	// collumn
//asm	mov ah,2
//asm	int	0x10
	}
	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
 }
 else
 {
	VL_SetTextMode();
	if (error && *error)
		puts(error);
 }
#endif

	exit(0);
}

//===========================================================================



/*
=====================
=
= DemoLoop
=
=====================
*/

static  char *ParmStrings[] = {"baby","easy","normal","hard",""};

void    DemoLoop (void)
{
	static int LastDemo;
	int     i,level;
	long nsize;
	memptr	nullblock;

//
// check for launch from ted
//
	if (tedlevel)
	{
		NoWait = true;
		NewGame(1,0);

		for (i = 1;i < _argc;i++)
		{
			if ( (level = US_CheckParm(_argv[i],ParmStrings)) != -1)
			{
			 gamestate.difficulty=level;
			 break;
			}
		}

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
		gamestate.mapon = tedlevelnum;
#elif (!defined SPEAR)
//#ifndef SPEAR
		gamestate.episode = tedlevelnum/10;
		gamestate.mapon = tedlevelnum%10;
#else
		gamestate.episode = 0;
		gamestate.mapon = tedlevelnum;
#endif
		GameLoop();
		Quit (NULL);
	}


//
// main game cycle
//


//	nsize = (long)40*1024;
//	MM_GetPtr(&nullblock,nsize);

#ifndef DEMOTEST

	#ifndef UPLOAD

		#ifndef GOODTIMES
		#ifndef SPEAR
		#ifndef JAPAN
		// *** S3DNA RESTORATION ***
		#ifndef GAMEVER_RESTORATION_N3D_WIS10
		if (!NoWait)
			NonShareware();
		#endif
		#endif
		#else

			#ifndef GOODTIMES
			#ifndef SPEARDEMO
			CopyProtection();
			#endif
			#endif

		#endif
		#endif
	#endif

// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_WL1_APO10
	displayofs = bufferofs = 0;
#endif
// *** PRE-V1.4 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_ANY_APO_PRE14
	StartCPMusic(INTROSONG);
#endif

// *** S3DNA RESTORATION ***
#if (!defined JAPAN) && (!defined GAMEVER_RESTORATION_N3D_WIS10)
//#ifndef JAPAN
	if (!NoWait)
		PG13 ();
#endif

#endif

	while (1)
	{
		while (!NoWait)
		{
//
// title page
//
			MM_SortMem ();
			// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_WL1_APO10
			bufferofs = 19200;
			displayofs = 0;
			VW_SetCRTC(displayofs);
#endif
#ifndef DEMOTEST

			// *** S3DNA RESTORATION ***
#if (defined SPEAR) || (defined GAMEVER_RESTORATION_N3D_WIS10)
//#ifdef SPEAR
			CA_CacheGrChunk (TITLEPALETTE);

			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
			CA_CacheScreen(TITLEPIC);
#else
			CA_CacheGrChunk (TITLE1PIC);
			VWB_DrawPic (0,0,TITLE1PIC);
			UNCACHEGRCHUNK (TITLE1PIC);

			CA_CacheGrChunk (TITLE2PIC);
			VWB_DrawPic (0,80,TITLE2PIC);
			UNCACHEGRCHUNK (TITLE2PIC);
#endif
			VW_UpdateScreen ();
			VL_FadeIn(0,255,grsegs[TITLEPALETTE],30);

			UNCACHEGRCHUNK (TITLEPALETTE);
#else
			CA_CacheScreen (TITLEPIC);
			// *** PRE-V1.4 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
			if (screenfaded)
			{
				VW_UpdateScreen ();
				VW_FadeIn();
			}
			else
			{
				if (FizzleFade(bufferofs,displayofs,320,200,20,true))
					break;
			}
#else
			VW_UpdateScreen ();
			VW_FadeIn();
#endif
#endif
			// *** PRE-V1.4 APOGEE + S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
			if (IN_UserInput(TickBase))
				break;
#elif (defined GAMEVER_RESTORATION_ANY_APO_PRE14)
			if (IN_UserInput(TickBase*3))
				break;
#else
			if (IN_UserInput(TickBase*15))
				break;
			VW_FadeOut();
#endif
			// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
			CA_CacheGrChunk (COPYRIGHTPIC);
			VWB_DrawPic (0,184,COPYRIGHTPIC);
			VW_UpdateScreen ();
			CA_LoadAllSounds ();
			UNCACHEGRCHUNK (COPYRIGHTPIC);
			if (IN_UserInput(TickBase))
				break;

			CA_CacheGrChunk (FAMILYAPIC);
			VWB_DrawPic (224,112,FAMILYAPIC);
			VW_UpdateScreen ();
			UNCACHEGRCHUNK (FAMILYAPIC);
			if (IN_UserInput(TickBase*13))
				break;

			VW_FadeOut ();
			Briefing (6,1);
#endif
			// *** S3DNA RESTORATION ***
#ifndef GAMEVER_RESTORATION_N3D_WIS10
//
// credits page
//
			CA_CacheScreen (CREDITSPIC);
#endif
			// *** PRE-V1.4 APOGEE RESTORATION ***
			// A bit of additional restored code
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
			if (FizzleFade(bufferofs,displayofs,320,200,20,true))
				break;
			if (IN_UserInput(TickBase*10))
				break;
#endif
			// *** REGISTERED SOD V1.4 ACTIVISION RESTORATION ***
			// Different restored code
#if (defined SPEAR) && (!defined GAMEVER_RESTORATION_ANY_PRE_GT)
			VW_ScreenToScreen (bufferofs,bufferofs+ylookup[175],30,17);
			VW_ScreenToScreen (bufferofs+ylookup[168]+30,displayofs,50,30);
			VW_ScreenToScreen (bufferofs+ylookup[42],bufferofs+ylookup[170]+30,20,30);
			VW_ScreenToScreen (bufferofs+ylookup[42],bufferofs+ylookup[165]+53,25,30);
			VW_ScreenToScreen (displayofs,bufferofs+ylookup[168]+18,50,30);
#endif
			// *** PRE-V1.4 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_ANY_APO_PRE14
			VW_UpdateScreen();
			VW_FadeIn ();
			if (IN_UserInput(TickBase*10))
				break;
			VW_FadeOut ();
//
// high scores
//
			DrawHighScores ();
			VW_UpdateScreen ();
			VW_FadeIn ();

			if (IN_UserInput(TickBase*10))
				break;
#endif // GAMEVER_RESTORATION_ANY_APO_PRE14
#endif
//
// demo
//

			// *** S3DNA RESTORATION ***
			#ifdef GAMEVER_RESTORATION_N3D_WIS10
			PlayDemo (LastDemo++%3);
			#elif (!defined SPEARDEMO)
			//#ifndef SPEARDEMO
			PlayDemo (LastDemo++%4);
			#else
			PlayDemo (0);
			#endif

			if (playstate == ex_abort)
				break;
			// *** PRE-V1.4 APOGEE RESTORATION ***
#ifdef GAMEVER_RESTORATION_ANY_APO_PRE14
			StartCPMusic(ROSTER_MUS);

			DrawHighScores();
			VW_UpdateScreen();
			VW_FadeIn();

			if (IN_UserInput(TickBase*9))
				break;
#endif
			StartCPMusic(INTROSONG);

		}

		VW_FadeOut ();

		// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
		if (Keyboard[sc_M])
			DoJukebox();
#endif
		// *** SHAREWARE V1.0 APOGEE RESTORATION ***
#ifndef SPEAR
		if (Keyboard[sc_Tab] && MS_CheckParm(GAMEVER_RESTORATION_W3D_DEBUGPARM))
#else
		if (Keyboard[sc_Tab] && MS_CheckParm("debugmode"))
#endif
			RecordDemo ();
		else
			US_ControlPanel (0);

		if (startgame || loadedgame)
		{
			GameLoop ();
			// *** PRE-V1.4 APOGEE RESTORATION ***
#ifndef GAMEVER_RESTORATION_ANY_APO_PRE14
			VW_FadeOut();
			StartCPMusic(INTROSONG);
#endif
		}
	}
}


//===========================================================================


/*
==========================
=
= main
=
==========================
*/

// *** S3DNA RESTORATION ***
#ifndef GAMEVER_RESTORATION_N3D_WIS10
char    *nosprtxt[] = {"nospr",nil};
#endif

void main (void)
{
	int     i;


#ifdef BETA
	//
	// THIS IS FOR BETA ONLY!
	//
	struct dosdate_t d;

	_dos_getdate(&d);
	if (d.year > YEAR ||
		(d.month >= MONTH && d.day >= DAY))
	{
	 printf("Sorry, BETA-TESTING is over. Thanks for you help.\n");
	 exit(1);
	}
#endif

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	vgamodeset = true;
	VL_SetTextMode();
	VL_WriteTextCharsWithAttr(' ',0x5d,80);
	gotoxy((79-strlen(titletxt))/2,1);
	puts(titletxt);
#endif

	CheckForEpisodes();

	Patch386 ();

	InitGame ();

	DemoLoop();

	// *** S3DNA RESTORATION ***
#ifdef GAMEVER_RESTORATION_N3D_WIS10
	Quit("FatalError: DemoLoop() exited!");
#else
	Quit("Demo loop exited???");
#endif
}

// *** SHAREWARE V1.0 APOGEE RESTORATION *** - Some unused function
#ifdef GAMEVER_RESTORATION_WL1_APO10
long GetRandomTableSum (void)
{
	extern far byte rndtable[];
	long result = 0;
	int i;
	for (i=0;i<0x100;i++)
		result += rndtable[i];
	return result;
}
#endif
