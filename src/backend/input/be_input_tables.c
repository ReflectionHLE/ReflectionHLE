#include <assert.h>
#include "be_cross.h"
#include "be_input_tables.h"

// Used for mapping cfg-side key identifiers to internal integers.
// It was originally made using SDL2 scancodes as a base.

struct {
	const char *name;
	int id;
} g_be_keyPairs[] = {
	{"", 0}, // Internally used by the launcher and cfg file reader
	{"A", 4},
	{"B", 5},
	{"C", 6},
	{"D", 7},
	{"E", 8},
	{"F", 9},
	{"G", 10},
	{"H", 11},
	{"I", 12},
	{"J", 13},
	{"K", 14},
	{"L", 15},
	{"M", 16},
	{"N", 17},
	{"O", 18},
	{"P", 19},
	{"Q", 20},
	{"R", 21},
	{"S", 22},
	{"T", 23},
	{"U", 24},
	{"V", 25},
	{"W", 26},
	{"X", 27},
	{"Y", 28},
	{"Z", 29},

	{"1", 30},
	{"2", 31},
	{"3", 32},
	{"4", 33},
	{"5", 34},
	{"6", 35},
	{"7", 36},
	{"8", 37},
	{"9", 38},
	{"0", 39},

	{"Enter", 40},
//	{"Esc", 41},
	{"Backspace", 42},
	{"Tab", 43},
	{"Space", 44},

	{"-", 45},
	{"=", 46},
	{"LBracket", 47},
	{"RBracket", 48},
	{"\\", 49},

	{";", 51},
	{"'", 52},
	{"`", 53},
	{",", 54},
	{".", 55},
	{"/", 56},

	{"CapsLock", 57},

	{"F1", 58},
	{"F2", 59},
	{"F3", 60},
	{"F4", 61},
	{"F5", 62},
	{"F6", 63},
	{"F7", 64},
	{"F8", 65},
	{"F9", 66},
	{"F10", 67},
	{"F11", 68},
	{"F12", 69},

	{"ScrollLock", 71},

	{"Insert", 73},
	{"Home", 74},
	{"PgUp", 75},
	{"Del", 76},
	{"End", 77},
	{"PgDn", 78},

	{"Right", 79},
	{"Left", 80},
	{"Down", 81},
	{"Up", 82},

	{"NumLock", 83},

	{"KP/", 84},
	{"KP*", 85},
	{"KP-", 86},
	{"KP+", 87},
	{"KPEnter", 88},
	{"KP1", 89},
	{"KP2", 90},
	{"KP3", 91},
	{"KP4", 92},
	{"KP5", 93},
	{"KP6", 94},
	{"KP7", 95},
	{"KP8", 96},
	{"KP9", 97},
	{"KP0", 98},
	{"KP.", 99},

	{"LessThan", 100}, // A secondary backslash key in certain layouts

	{"LCtrl", 224},
	{"LShift", 225},
	{"LAlt", 226},

	{"RCtrl", 228},
	{"RShift", 229},
	{"RAlt", 230},
};

const char *g_be_st_keyIdToNameMap[BE_MAX_KEY_ID];

// Enumarated by SDL mouses buttons, for most
const char *g_be_st_mouseFeatureIdToNameMap[1+BE_ST_CTRL_MOUSE_BUT_MAX] = {
	"Left", "Middle", "Right", "X1", "X2",
	"", // Used for internal default
	NULL // Used as a terminator for the launcher
};

void BEL_ST_InitKeyMap(void)
{
	for (unsigned i = 0; i < BE_Cross_ArrayLen(g_be_keyPairs); ++i)
	{
		assert(g_be_keyPairs[i].id < BE_MAX_KEY_ID);
		g_be_st_keyIdToNameMap[g_be_keyPairs[i].id] = g_be_keyPairs[i].name;
	}
}
