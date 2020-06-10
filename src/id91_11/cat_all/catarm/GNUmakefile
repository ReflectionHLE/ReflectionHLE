VERDIRNAME=catarm
EXENAME=reflection-catarm
SRC=.
BESRC=../../..
RSRC=../../../../rsrc

C3DCOMMONSRC=..
CATLATECOMMONSRC=../cat_late
IDSRC=../..
EXTSRC=../external_restored
IDOBJ=$(OBJ)/id
EXTOBJ=$(OBJ)/ext

GAMEOBJECTS=$(OBJ)/c5_act1.o \
            $(OBJ)/c5_act2.o \
            $(OBJ)/c5_act3.o \
            $(OBJ)/c5_act4.o \
            $(OBJ)/c5_game.o \
            $(OBJ)/c5_main.o \
            $(OBJ)/c5_play.o \
            $(OBJ)/c5_state.o \
            $(OBJ)/c5_wiz.o \
            $(OBJ)/statetype_ptr_conversion.o

#Shared with The Catacomb Armageddon and Apocalypse
GAMEOBJECTS+= $(OBJ)/c5_debug.o \
              $(OBJ)/c5_draw.o \
              $(OBJ)/gelib.o

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
           $(EXTOBJ)/slidecat.o \
           $(EXTOBJ)/intro.o

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

OBJECTS=$(GAMEOBJECTS) $(IDOBJECTS) $(EXTOBJECTS) $(BEOBJECTS)

INTCXXFLAGS+= -Wno-char-subscripts -I$(C3DCOMMONSRC) -I$(CATLATECOMMONSRC) -I$(IDSRC) -I$(EXTSRC) -DREFKEEN_VER_CATADVENTURES=1 -DREFKEEN_HAS_VER_CATADVENTURES=1 -DREFKEEN_VER_CATARM=1 -DREFKEEN_HAS_VER_CATARM=1 -DREFKEEN_VER_CATACOMB_ALL=1 -DREFKEEN_HAS_VER_CATACOMB_ALL=1

include ../../../GNUmakefile.common

$(EXTOBJ)/%.o: $(EXTSRC)/%.c $(EXTOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(IDOBJ)/%.o: $(IDSRC)/%.c $(IDOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(OBJ)/%.o: $(CATLATECOMMONSRC)/%.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(OBJ)/%.o: $(C3DCOMMONSRC)/%.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(EXTOBJ): | $(OBJ)
	-mkdir $(EXTOBJ)
$(IDOBJ): | $(OBJ)
	-mkdir $(IDOBJ)

