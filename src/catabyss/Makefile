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
SRC=.
BESRC=..
OBJ=obj
BEOBJ=$(OBJ)/be

VERSRC=$(SRC)/catabshar113

# HACK (Kind of)
RSRCSRC=$(SRC)/static_shareware
RSRCOBJ=$(OBJ)/static

OBJECTS=$(OBJ)/c4_act1.o \
        $(OBJ)/c4_asm.o \
        $(OBJ)/c4_debug.o \
        $(OBJ)/c4_draw.o \
        $(OBJ)/c4_game.o \
        $(OBJ)/c4_main.o \
        $(OBJ)/c4_play.o \
        $(OBJ)/c4_scale.o \
        $(OBJ)/c4_state.o \
        $(OBJ)/c4_trace.o \
        $(OBJ)/c4_wiz.o \
        $(OBJ)/gelib.o \
        $(OBJ)/id_ca.o \
        $(OBJ)/id_in.o \
        $(OBJ)/id_mm.o \
        $(OBJ)/id_sd.o \
        $(OBJ)/id_us_1.o \
        $(OBJ)/id_us_a.o \
        $(OBJ)/id_vw_a.o \
        $(OBJ)/id_vw_ae.o \
        $(OBJ)/id_vw.o

BEOBJECTS=$(BEOBJ)/actual_main.o \
          $(BEOBJ)/be_cross.o \
          $(BEOBJ)/be_cross_compat.o \
          $(BEOBJ)/be_sdl.o \
          $(BEOBJ)/be_sdl_audio_timer.o \
          $(BEOBJ)/be_sdl_graphics.o \
          $(BEOBJ)/be_textmode_fonts.o \
          $(BEOBJ)/dbopl.o

#NOTE: Unnecessary resources may be omitted
RSRC_OBJECTS=$(RSRCOBJ)/audiodct.o \
             $(RSRCOBJ)/audiohhd.o \
             $(RSRCOBJ)/egadict.o \
             $(RSRCOBJ)/egahead.o \
             $(RSRCOBJ)/mtemp.o

INTCXXFLAGS=-I$(SRC) -I$(VERSRC) -I$(BESRC) -DCHOCO_KEEN_VER_CATABYSS=1

EXENAME=chocolate-catabyss

ifeq ($(DEBUG),1)
	INTCXXFLAGS+= -ggdb -ftrapv -fstack-check -DCHOCOLATE_KEEN_CONFIG_DEBUG
else
	INTCXXFLAGS+= -O2
endif

INTCXXFLAGS+= `$(SDLCONFIG) --cflags` -Wall -Wno-pointer-sign -Wno-unknown-pragmas -Wno-unused-variable -Wno-missing-braces -Wno-switch
#We need -lm for dbopl
INTLDFLAGS=`$(SDLCONFIG) --libs` -lm

#FIXME Using gnu99 instead of c99 due to scandir and alphasort
ifeq ($(BUILDASCPP), 0)
	INTCXXFLAGS+= -std=gnu99
endif

ifeq ($(PLATFORM), WINDOWS)
	EXE_EXT=.exe
	INTCXXFLAGS+= -mno-ms-bitfields #To make __attribute__((__packed__)) work...
endif

EXE_PATH=../$(EXENAME)$(EXE_EXT)

.PHONY: all game clean veryclean

all: game

game: $(EXE_PATH)

$(EXE_PATH): $(OBJECTS) $(BEOBJECTS) $(RSRC_OBJECTS)
	$(CXX) $(OBJECTS) $(BEOBJECTS) $(RSRC_OBJECTS) $(LDFLAGS) $(INTLDFLAGS) -o $@
ifeq ($(DEBUG),0)
	$(STRIPBIN) $(EXE_PATH)
endif

$(BEOBJ)/dbopl.o: $(BESRC)/opl/dbopl.c | $(BEOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(BEOBJ)/be_cross_compat.o: $(VERSRC)/be_cross_compat.c | $(BEOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(BEOBJ)/%.o: $(BESRC)/%.c $(BEOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(RSRCOBJ)/%.o: $(RSRCSRC)/%.c | $(RSRCOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(RSRCOBJ): | $(OBJ)
	-mkdir $(RSRCOBJ)
$(BEOBJ): | $(OBJ)
	-mkdir $(BEOBJ)
$(OBJ):
	-mkdir $(OBJ)

clean:
	-rm -f $(EXE_PATH) $(OBJECTS) $(BEOBJECTS) $(RSRC_OBJECTS)

