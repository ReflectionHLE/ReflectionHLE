VERDIRNAME=kdreams
EXENAME=reflection-kdreams
SRC=.
BESRC=..
RSRC=../../rsrc

include ../Makefile.common

OBJECTS=$(OBJ)/id_ca.o \
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
        $(OBJ)/altcontroller.o \
        $(OBJ)/statetype_ptr_conversion.o

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

INTCXXFLAGS+= -DREFKEEN_VER_KDREAMS=1

.PHONY: all game clean veryclean

all: game

game: $(EXE_PATH)

$(EXE_PATH): $(OBJECTS) $(BEOBJECTS)
	$(CXX) $(OBJECTS) $(BEOBJECTS) $(LDFLAGS) $(INTLDFLAGS) -o $@
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

$(OBJ)/loadscn2.o: $(SRC)/lscr/loadscn2.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(BEOBJ)/%.o: $(BESRC)/%.c $(BEOBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@
$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CXX) -c $(INTCXXFLAGS) $(CXXFLAGS) $< -o $@

$(BEOBJ): | $(OBJ)
	-mkdir $(BEOBJ)
$(OBJ): | $(OBJPREFIX)
	-mkdir $(OBJ)
$(OBJPREFIX):
	-mkdir $(OBJPREFIX)

clean:
	-rm -f $(EXE_PATH) $(OBJECTS) $(BEOBJECTS)

