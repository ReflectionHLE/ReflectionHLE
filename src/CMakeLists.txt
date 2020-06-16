list(APPEND be_src
	src/actual_main.c
	src/be_cross.c
	src/be_cross_doszeroseg.c
	src/be_cross_fs.c
	src/be_cross_mem.c
	src/be_st_sdl_audio_timer.c
	src/be_st_sdl.c
	src/be_st_sdl_graphics.c
	src/be_textmode_fonts.c
	src/crc32/crc32.c
	src/depklite/depklite.c
	src/opl/dbopl.c
	src/unlzexe/unlzexe.c
)

if (REFKEEN_ENABLE_LAUNCHER)
	list(APPEND be_src src/be_launcher.c src/be_st_sdl_launcher.c)
endif (REFKEEN_ENABLE_LAUNCHER)
