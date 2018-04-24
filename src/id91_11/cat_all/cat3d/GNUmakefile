VERDIRNAME=cat3d
EXENAME=reflection-cat3d
SRC=.
BESRC=../../..
RSRC=../../../../rsrc

C3DCOMMONSRC=..
IDSRC=../..
IDOBJ=$(OBJ)/id

GAMEOBJECTS=$(OBJ)/c3_act1.o \
            $(OBJ)/c3_debug.o \
            $(OBJ)/c3_draw.o \
            $(OBJ)/c3_game.o \
            $(OBJ)/c3_main.o \
            $(OBJ)/c3_play.o \
            $(OBJ)/c3_state.o \
            $(OBJ)/c3_wiz.o \
            $(OBJ)/statetype_ptr_conversion.o

#Shared with Catacomb 3-D and The Adventures Series
GAMEOBJECTS+= $(OBJ)/c3_asm.o \
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
          $(BEOBJ)/be_cross_mem.o \
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

OBJECTS=$(GAMEOBJECTS) $(IDOBJECTS) $(BEOBJECTS)

INTCXXFLAGS+= -I$(C3DCOMMONSRC) -I$(IDSRC) -DREFKEEN_VER_CAT3D=1 -DREFKEEN_VER_CATACOMB_ALL=1

include ../../../GNUmakefile.common

$(IDOBJ)/%.o: $(IDSRC)/%.c $(IDOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(OBJ)/%.o: $(C3DCOMMONSRC)/%.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(IDOBJ): | $(OBJ)
	-mkdir $(IDOBJ)

