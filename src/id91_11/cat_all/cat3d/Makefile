VERDIRNAME=cat3d
EXENAME=reflection-cat3d
SRC=.
BESRC=../../..
RSRC=../../../../rsrc

include ../../../Makefile.common

C3DCOMMONSRC=..
IDSRC=../..
IDOBJ=$(OBJ)/id

OBJECTS=$(OBJ)/c3_act1.o \
        $(OBJ)/c3_debug.o \
        $(OBJ)/c3_draw.o \
        $(OBJ)/c3_game.o \
        $(OBJ)/c3_main.o \
        $(OBJ)/c3_play.o \
        $(OBJ)/c3_state.o \
        $(OBJ)/c3_wiz.o \
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
          $(IDOBJ)/id_us_2.o \
          $(IDOBJ)/id_us_a.o \
          $(IDOBJ)/id_vw_a.o \
          $(IDOBJ)/id_vw_ae.o \
          $(IDOBJ)/id_vw.o

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

INTCXXFLAGS+= -I$(C3DCOMMONSRC) -I$(IDSRC) -DREFKEEN_VER_CAT3D=1 -DREFKEEN_VER_CATACOMB_ALL=1

.PHONY: all game clean veryclean

all: game

game: $(EXE_PATH)

$(EXE_PATH): $(OBJECTS) $(IDOBJECTS) $(BEOBJECTS)
	$(CXX) $(OBJECTS) $(IDOBJECTS) $(BEOBJECTS) $(LDFLAGS) $(INTLDFLAGS) -o $@
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
$(IDOBJ)/%.o: $(IDSRC)/%.c $(IDOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(OBJ)/%.o: $(C3DCOMMONSRC)/%.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(BEOBJ): | $(OBJ)
	-mkdir $(BEOBJ)
$(IDOBJ): | $(OBJ)
	-mkdir $(IDOBJ)
$(OBJ): | $(OBJPREFIX)
	-mkdir $(OBJ)
$(OBJPREFIX):
	-mkdir $(OBJPREFIX)

clean:
	-rm -f $(EXE_PATH) $(OBJECTS) $(IDOBJECTS) $(BEOBJECTS)

