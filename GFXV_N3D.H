//////////////////////////////////////
//
// Graphics .H file for .N3D
// Recreated using ECWolf definitions
//
//////////////////////////////////////

typedef enum {
		// Lump Start
		H_TOPWINDOWPIC=3,
		H_LEFTWINDOWPIC,                     // 4
		H_RIGHTWINDOWPIC,                    // 5
		H_BOTTOMINFOPIC,                     // 6
		// Lump Start
		C_OPTIONSPIC,                        // 7
		C_CURSOR1PIC,                        // 8
		C_CURSOR2PIC,                        // 9
		C_NOTSELECTEDPIC,                    // 10
		C_SELECTEDPIC,                       // 11
		C_FXTITLEPIC,                        // 12
		C_DIGITITLEPIC,                      // 13
		C_MUSICTITLEPIC,                     // 14
		C_MOUSELBACKPIC,                     // 15
		C_DISKLOADING1PIC,                   // 16
		C_DISKLOADING2PIC,                   // 17
		C_DISKLOADING3PIC,                   // 18
		C_DISKLOADING4PIC,                   // 19
		C_CONTROLPIC,                        // 20
		C_BABYMODEPIC,                       // 21
		C_EASYPIC,                           // 22
		C_NORMALPIC,                         // 23
		C_HARDPIC,                           // 24
		C_CUSTOMIZEPIC,                      // 25
		C_LOADGAMEPIC,                       // 26
		C_SAVEGAMEPIC,                       // 27
		C_EPISODE1PIC,                       // 28
		C_EPISODE2PIC,                       // 29
		C_EPISODE3PIC,                       // 30
		C_EPISODE4PIC,                       // 31
		C_EPISODE5PIC,                       // 32
		C_EPISODE6PIC,                       // 33
		C_LEVELPIC,                          // 34
		C_NAMEPIC,                           // 35
		C_SCOREPIC,                          // 36
		C_JOY1PIC,                           // 37
		C_JOY2PIC,                           // 38
		// Lump Start
		L_GUYPIC,                            // 39
		L_GUY2PIC,                           // 40
		L_BJWINSPIC,                         // 41
		L_COLONPIC,                          // 42
		L_NUM0PIC,                           // 43
		L_NUM1PIC,                           // 44
		L_NUM2PIC,                           // 45
		L_NUM3PIC,                           // 46
		L_NUM4PIC,                           // 47
		L_NUM5PIC,                           // 48
		L_NUM6PIC,                           // 49
		L_NUM7PIC,                           // 50
		L_NUM8PIC,                           // 51
		L_NUM9PIC,                           // 52
		L_PERCENTPIC,                        // 53
		L_MINUSPIC,                          // 54
		L_EXPOINTPIC,                        // 55
		L_APOSTROPHEPIC,                     // 56
		Y_NUM0PIC,                           // 57
		Y_NUM1PIC,                           // 58
		Y_NUM2PIC,                           // 59
		Y_NUM3PIC,                           // 60
		Y_NUM4PIC,                           // 61
		Y_NUM5PIC,                           // 62
		Y_NUM6PIC,                           // 63
		Y_NUM7PIC,                           // 64
		Y_NUM8PIC,                           // 65
		Y_NUM9PIC,                           // 66
		W_BONUSPIC,                          // 67
		W_PERFECTPIC,                        // 68
		W_RATINGPIC,                         // 69
		W_LEVELPIC,                          // 70
		W_FLOORPIC,                          // 71
		W_FINISHPIC,                         // 72
		W_KILLSPIC,                          // 73
		W_TREASURESPIC,                      // 74
		W_SECRETSPIC,                        // 75
		W_TIMEPIC,                           // 76
		W_PARPIC,                            // 77
		W_SECRETPIC,                         // 78
		STATUSBARPIC,                        // 79
		TITLEPIC,                            // 80
		FAMILYAPIC,                          // 81
		COPYRIGHTPIC,                        // 82
		HIGHSCORESPIC,                       // 83
		// Lump Start
		NOKEYPIC,                            // 84
		GOLDKEYPIC,                          // 85
		SILVERKEYPIC,                        // 86
		N_BLANKPIC,                          // 87
		N_0PIC,                              // 88
		N_1PIC,                              // 89
		N_2PIC,                              // 90
		N_3PIC,                              // 91
		N_4PIC,                              // 92
		N_5PIC,                              // 93
		N_6PIC,                              // 94
		N_7PIC,                              // 95
		N_8PIC,                              // 96
		N_9PIC,                              // 97
		FACE1APIC,                           // 98
		FACE1BPIC,                           // 99
		FACE1CPIC,                           // 100
		FACE2APIC,                           // 101
		FACE2BPIC,                           // 102
		FACE2CPIC,                           // 103
		FACE3APIC,                           // 104
		FACE3BPIC,                           // 105
		FACE3CPIC,                           // 106
		FACE4APIC,                           // 107
		FACE4BPIC,                           // 108
		FACE4CPIC,                           // 109
		FACE5APIC,                           // 110
		FACE5BPIC,                           // 111
		FACE5CPIC,                           // 112
		FACE6APIC,                           // 113
		FACE6BPIC,                           // 114
		FACE6CPIC,                           // 115
		FACE7APIC,                           // 116
		FACE7BPIC,                           // 117
		FACE7CPIC,                           // 118
		FACE8APIC,                           // 119
		GODMODEFACE1PIC,                     // 120
		GODMODEFACE2PIC,                     // 121
		GODMODEFACE3PIC,                     // 122
		GOTGATLINGPIC,                       // 123
		PAUSEDPIC,                           // 124
		GETPSYCHEDPIC,                       // 125



		TITLEPALETTE=127,
		ORDERSCREEN,                         // 128
		ERRORSCREEN,                         // 129
		T_HELPART,                           // 130
		T_DEMO0,                             // 131
		T_DEMO1,                             // 132
		T_DEMO2,                             // 133
		ENUMEND
	     } graphicnums;

//
// Data LUMPs
//
#define README_LUMP_START		3
#define README_LUMP_END			6

#define CONTROLS_LUMP_START		7
#define CONTROLS_LUMP_END		38

#define LEVELEND_LUMP_START		39
#define LEVELEND_LUMP_END		78

#define LATCHPICS_LUMP_START		84
#define LATCHPICS_LUMP_END		125


//
// Amount of each data item
//
#define NUMCHUNKS    134
#define NUMFONT      2
#define NUMFONTM     0
#define NUMPICS      123
#define NUMPICM      0
#define NUMSPRITES   0
#define NUMTILE8     72
#define NUMTILE8M    0
#define NUMTILE16    0
#define NUMTILE16M   0
#define NUMTILE32    0
#define NUMTILE32M   0
#define NUMEXTERNS   8
//
// File offsets for data items
//
#define STRUCTPIC    0

#define STARTFONT    1
#define STARTFONTM   3
#define STARTPICS    3
#define STARTPICM    126
#define STARTSPRITES 126
#define STARTTILE8   126
#define STARTTILE8M  127
#define STARTTILE16  127
#define STARTTILE16M 127
#define STARTTILE32  127
#define STARTTILE32M 127
#define STARTEXTERNS 127

//
// Thank you for using IGRAB!
//
