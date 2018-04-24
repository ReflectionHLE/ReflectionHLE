VERDIRNAME=kdreams
EXENAME=reflection-kdreams
SRC=.
BESRC=..
RSRC=../../rsrc

GAMEOBJECTS=$(OBJ)/id_ca.o \
            $(OBJ)/id_in.o \
            $(OBJ)/id_mm.o \
            $(OBJ)/id_rf_a.o \
            $(OBJ)/id_rf.o \
            $(OBJ)/id_sd.o \
            $(OBJ)/id_us_a.o \
            $(OBJ)/id_us.o \
            $(OBJ)/id_vw_a.o \
            $(OBJ)/id_vw_ac.o \
            $(OBJ)/id_vw_ae.o \
            $(OBJ)/id_vw.o \
            $(OBJ)/kd_act1.o \
            $(OBJ)/kd_act2.o \
            $(OBJ)/kd_demo.o \
            $(OBJ)/kd_keen.o \
            $(OBJ)/kd_main.o \
            $(OBJ)/kd_play.o \
            $(OBJ)/gelib.o \
            $(OBJ)/jam_io.o \
            $(OBJ)/loadscn2.o \
            $(OBJ)/lzhuf.o \
            $(OBJ)/soft.o \
            $(OBJ)/key_scancode_converters.o \
            $(OBJ)/altcontroller.o \
            $(OBJ)/statetype_ptr_conversion.o

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
          $(BEOBJ)/depklite.o \
          $(BEOBJ)/unlzexe.o

OBJECTS=$(GAMEOBJECTS) $(BEOBJECTS)

INTCXXFLAGS+= -DREFKEEN_VER_KDREAMS=1

include ../Makefile.common

$(OBJ)/loadscn2.o: $(SRC)/lscr/loadscn2.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

