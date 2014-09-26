DEBUG=0
BUILDASCPP=0
BINPREFIX=

ifeq ($(BUILDASCPP), 1)
	CXX=$(BINPREFIX)g++
else
	CXX=$(BINPREFIX)gcc
endif

STRIPBIN=$(BINPREFIX)strip
SDLCONFIG=sdl2-config
SRC=src
OBJ=obj
RSRCSRC=$(SRC)/static
RSRCOBJ=$(OBJ)/static

OBJECTS=$(OBJ)/actual_main.o \
        $(OBJ)/be_cross.o \
        $(OBJ)/be_sdl.o \
        $(OBJ)/be_sdl_audio_timer.o \
        $(OBJ)/be_sdl_graphics.o \
        $(OBJ)/be_textmode_fonts.o \
        $(OBJ)/dbopl.o \
        $(OBJ)/id_ca.o \
        $(OBJ)/id_in.o \
        $(OBJ)/id_mm.o \
        $(OBJ)/id_rf_a.o \
        $(OBJ)/id_rf.o \
        $(OBJ)/id_sd.o \
        $(OBJ)/id_us_a.o \
        $(OBJ)/id_us.o \
        $(OBJ)/id_vw_a.o \
        $(OBJ)/id_vw_ac.o \
        $(OBJ)/id_vw.o \
        $(OBJ)/kd_act1.o \
        $(OBJ)/kd_act2.o \
        $(OBJ)/kd_demo.o \
        $(OBJ)/kd_keen.o \
        $(OBJ)/kd_main.o \
        $(OBJ)/kd_play.o

#CURRENTLY OMITTED: egadict, egahead, piracy
RSRC_OBJECTS=$(RSRCOBJ)/audiodct.o \
             $(RSRCOBJ)/audiohhd.o \
             $(RSRCOBJ)/cgadict.o \
             $(RSRCOBJ)/cgahead.o \
             $(RSRCOBJ)/context.o \
             $(RSRCOBJ)/gametext.o \
             $(RSRCOBJ)/mapdict.o \
             $(RSRCOBJ)/maphead.o \
             $(RSRCOBJ)/story.o


INTCXXFLAGS=

ifeq ($(DEBUG),1)
	INTCXXFLAGS+= -ggdb -ftrapv -fstack-check -DCHOCOLATE_KEEN_CONFIG_DEBUG
else
	INTCXXFLAGS+= -O2
endif

INTCXXFLAGS+= -I$(SRC) `$(SDLCONFIG) --cflags` -Wall -Wno-pointer-sign -Wno-unknown-pragmas -Wno-unused-variable
#We need -lm for dbopl
INTLDFLAGS=`$(SDLCONFIG) --libs` -lm

ifeq ($(BUILDASCPP), 0)
	INTCXXFLAGS+= -std=c99
endif

ifeq ($(PLATFORM), WINDOWS)
	EXE_EXT=.exe
	INTCXXFLAGS+= -mno-ms-bitfields #To make __attribute__((__packed__)) work...
endif

EXE_PATH=chocolate-keen-dreams-cga$(EXE_EXT)

.PHONY: all game clean veryclean

all: game

game: $(EXE_PATH)

$(EXE_PATH): $(OBJECTS) $(RSRC_OBJECTS)
	$(CXX) $(OBJECTS) $(RSRC_OBJECTS) $(LDFLAGS) $(INTLDFLAGS) -o $@
ifeq ($(DEBUG),0)
	$(STRIPBIN) $(EXE_PATH)
endif

$(EXE_PATH): $(OBJECTS) $(RSRC_OBJECTS)

$(OBJ)/dbopl.o: $(SRC)/opl/dbopl.c
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(OBJ)/%.o: $(SRC)/%.c
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(RSRCOBJ)/%.o: $(RSRCSRC)/%.c
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
veryclean:
	-rm -f $(OBJ)/* $(RSRCOBJ)/* $(EXE_PATH)
clean:
	-rm -f $(EXE_PATH) $(OBJECTS) $(RSRC_OBJECTS)

