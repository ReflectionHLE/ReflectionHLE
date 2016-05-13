VERDIRNAME=catabyss
EXENAME=reflection-catabyss
SRC=.
BESRC=../../..
RSRC=../../../../rsrc

include ../../../Makefile.common

C3DCOMMONSRC=..
IDSRC=../..
EXTSRC=../external_restored
IDOBJ=$(OBJ)/id
EXTOBJ=$(OBJ)/ext

OBJECTS=$(OBJ)/c4_act1.o \
        $(OBJ)/c4_debug.o \
        $(OBJ)/c4_draw.o \
        $(OBJ)/c4_game.o \
        $(OBJ)/c4_main.o \
        $(OBJ)/c4_play.o \
        $(OBJ)/c4_state.o \
        $(OBJ)/c4_wiz.o \
        $(OBJ)/gelib.o \
        $(OBJ)/statetype_ptr_conversion.o

#Shared with Catacomb 3-D and The Adventures Series
OBJECTS+= $(OBJ)/c3_asm.o \
          $(OBJ)/c3_scale.o \
          $(OBJ)/c3_trace.o \
          $(OBJ)/altcontroller.o

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
           $(EXTOBJ)/loadscn.o \
           $(EXTOBJ)/intro.o

BEOBJECTS=$(BEOBJ)/actual_main.o \
          $(BEOBJ)/be_cross.o \
          $(BEOBJ)/be_cross_fs.o \
          $(BEOBJ)/be_cross_doszeroseg.o \
          $(BEOBJ)/be_launcher.o \
          $(BEOBJ)/be_st_sdl.o \
          $(BEOBJ)/be_st_sdl_audio_timer.o \
          $(BEOBJ)/be_st_sdl_graphics.o \
          $(BEOBJ)/be_st_sdl_launcher.o \
          $(BEOBJ)/be_textmode_fonts.o \
          $(BEOBJ)/crc32.o \
          $(BEOBJ)/dbopl.o \
          $(BEOBJ)/unlzexe.o

INTCXXFLAGS+= -Wno-char-subscripts -I$(C3DCOMMONSRC) -I$(IDSRC) -I$(EXTSRC) -DREFKEEN_VER_CATADVENTURES=1 -DREFKEEN_VER_CATABYSS=1 -DREFKEEN_VER_CATACOMB_ALL=1

.PHONY: all game clean veryclean

all: game

game: $(EXE_PATH)

$(EXE_PATH): $(OBJECTS) $(EXTOBJECTS) $(IDOBJECTS) $(BEOBJECTS)
	$(CXX) $(OBJECTS) $(EXTOBJECTS) $(IDOBJECTS) $(BEOBJECTS) $(LDFLAGS) $(INTLDFLAGS) -o $@
ifeq ($(DEBUG),0)
	$(STRIPBIN) $(EXE_PATH)
endif

$(RES_PATH): $(RSRC)/$(EXENAME).rc
	$(WINDRESBIN) $< -O coff -o $@

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

$(OBJ)/%.o: $(C3DCOMMONSRC)/%.c | $(OBJ)
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

