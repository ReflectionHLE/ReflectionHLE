// ID_HEAD.H


#define	EXTENSION	"WLF"
#define WOLF

#define	TEXTGR	0
#define	CGAGR	1
#define	EGAGR	2
#define	VGAGR	3

#define GRMODE	VGAGR

// *** S3DNA RESTORATION ***
// Unused, but fpos_t is found in the debugging symbols for ID_VL.C, which is
// the only place from which ID_HEAD.H is included, and it's mentioned in
// STDIO.H only (possibly done while adding the FillCharAttr function)
#ifdef GAMEVER_NOAH3D
#include <STDIO.H>
#endif
#include "VERSION.H"

typedef	enum	{false,true}	boolean;
typedef	unsigned	char		byte;
typedef	unsigned	int			word;
typedef	unsigned	long		longword;
typedef	byte *					Ptr;

typedef	struct
		{
			int	x,y;
		} Point;

typedef	struct
		{
			Point	ul,lr;
		} Rect;


void	Quit (char *error);		// defined in user program

