VERDIRNAME=catapoc

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
C3DCOMMONSRC=..
CATLATECOMMONSRC=../cat_late
IDSRC=../..
BESRC=../../..
EXTSRC=../external_restored
OBJPREFIX=obj
OBJ=$(OBJPREFIX)/$(VERDIRNAME)
IDOBJ=$(OBJ)/id
BEOBJ=$(OBJ)/be
EXTOBJ=$(OBJ)/ext

OBJECTS=$(OBJ)/c6_act1.o \
        $(OBJ)/c6_act2.o \
        $(OBJ)/c6_act3.o \
        $(OBJ)/c6_act4.o \
        $(OBJ)/c6_game.o \
        $(OBJ)/c6_main.o \
        $(OBJ)/c6_play.o \
        $(OBJ)/c6_state.o \
        $(OBJ)/c6_wiz.o \
        $(OBJ)/statetype_ptr_conversion.o

#Shared with The Catacomb Armageddon and Apocalypse
OBJECTS+= $(OBJ)/c5_debug.o \
          $(OBJ)/c5_draw.o \
          $(OBJ)/gelib.o

#Shared with Catacomb 3-D and The Adventures Series
OBJECTS+= $(OBJ)/c3_asm.o \
          $(OBJ)/c3_scale.o \
          $(OBJ)/c3_trace.o

IDOBJECTS=$(IDOBJ)/id_ca.o \
          $(IDOBJ)/id_in.o \
          $(IDOBJ)/id_mm.o \
          $(IDOBJ)/id_sd.o \
          $(IDOBJ)/id_us_1.o \
          $(IDOBJ)/id_us_a.o \
          $(IDOBJ)/id_vw_a.o \
          $(IDOBJ)/id_vw_ae.o \
          $(IDOBJ)/id_vw.o

EXTOBJECTS=$(EXTOBJ)/ext_ca.o \
           $(EXTOBJ)/ext_gelib.o \
           $(EXTOBJ)/ext_jam_io.o \
           $(EXTOBJ)/ext_lzhuf.o \
           $(EXTOBJ)/ext_soft.o \
           $(EXTOBJ)/ext_vw_a.o \
           $(EXTOBJ)/intro.o

BEOBJECTS=$(BEOBJ)/actual_main.o \
          $(BEOBJ)/be_cross.o \
          $(BEOBJ)/be_cross_fs.o \
          $(BEOBJ)/be_cross_doszeroseg.o \
          $(BEOBJ)/be_st_sdl.o \
          $(BEOBJ)/be_st_sdl_audio_timer.o \
          $(BEOBJ)/be_st_sdl_graphics.o \
          $(BEOBJ)/be_textmode_fonts.o \
          $(BEOBJ)/crc32.o \
          $(BEOBJ)/dbopl.o \
          $(BEOBJ)/unlzexe.o

INTCXXFLAGS=-I$(SRC) -I$(C3DCOMMONSRC) -I$(CATLATECOMMONSRC) -I$(IDSRC) -I$(EXTSRC) -I$(BESRC) -DREFKEEN_VER_CATADVENTURES=1 -DREFKEEN_VER_CATAPOC=1 -DREFKEEN_VER_CATACOMB_ALL=1

EXENAME=refcatapoc-101

ifeq ($(DEBUG),1)
	INTCXXFLAGS+= -ggdb -ftrapv -fstack-check -DREFKEEN_CONFIG_DEBUG
	CXXOPT=
else
	CXXOPT=-O2
endif

INTCXXFLAGS+= $(CXXOPT) `$(SDLCONFIG) --cflags` -Wall -Wno-missing-braces -Wno-switch -Wno-parentheses -Wno-unused-result -Wno-maybe-uninitialized -Wno-char-subscripts
#We need -lm for dbopl
INTLDFLAGS=`$(SDLCONFIG) --libs` -lm

ifeq ($(BUILDASCPP), 0)
	INTCXXFLAGS+= -std=c99
endif

ifeq ($(PLATFORM), WINDOWS)
	EXE_EXT=.exe
	INTCXXFLAGS+= -mno-ms-bitfields #To make __attribute__((__packed__)) work...
	#We need this for registry entry retrieval
	INTLDFLAGS+= -lshlwapi
endif

EXE_PATH=$(BESRC)/$(EXENAME)$(EXE_EXT)

.PHONY: all game clean veryclean

all: game

game: $(EXE_PATH)

$(EXE_PATH): $(OBJECTS) $(EXTOBJECTS) $(IDOBJECTS) $(BEOBJECTS)
	$(CXX) $(OBJECTS) $(EXTOBJECTS) $(IDOBJECTS) $(BEOBJECTS) $(LDFLAGS) $(INTLDFLAGS) -o $@
ifeq ($(DEBUG),0)
	$(STRIPBIN) $(EXE_PATH)
endif

$(BEOBJ)/crc32.o: $(BESRC)/crc32/crc32.c | $(BEOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(BEOBJ)/dbopl.o: $(BESRC)/opl/dbopl.c | $(BEOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(BEOBJ)/unlzexe.o: $(BESRC)/unlzexe/unlzexe.c | $(BEOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(BEOBJ)/%.o: $(BESRC)/%.c $(BEOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(EXTOBJ)/%.o: $(EXTSRC)/%.c $(EXTOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(IDOBJ)/%.o: $(IDSRC)/%.c $(IDOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(OBJ)/c5_debug.o: $(CATLATECOMMONSRC)/c5_debug.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(OBJ)/c5_draw.o: $(CATLATECOMMONSRC)/c5_draw.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(OBJ)/gelib.o: $(CATLATECOMMONSRC)/gelib.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(OBJ)/c3_asm.o: $(C3DCOMMONSRC)/c3_asm.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(OBJ)/c3_scale.o: $(C3DCOMMONSRC)/c3_scale.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(OBJ)/c3_trace.o: $(C3DCOMMONSRC)/c3_trace.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(EXTOBJ): | $(OBJ)
	-mkdir $(EXTOBJ)
$(BEOBJ): | $(OBJ)
	-mkdir $(BEOBJ)
$(IDOBJ): | $(OBJ)
	-mkdir $(IDOBJ)
$(OBJ): | $(OBJPREFIX)
	-mkdir $(OBJ)
$(OBJPREFIX):
	-mkdir $(OBJPREFIX)

clean:
	-rm -f $(EXE_PATH) $(OBJECTS) $(IDOBJECTS) $(EXTOBJECTS) $(BEOBJECTS)

