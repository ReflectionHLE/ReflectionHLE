// Direct accesses to any of these functions should be minimized

/*** General ***/
void BE_SDL_InitAll(void);
void BE_SDL_ShutdownAll(void);
void BE_SDL_StartKeyboardService(void (*funcPtr)(id0_byte_t));
void BE_SDL_StopKeyboardService(void);
void BE_SDL_PollEvents(void);

/*** Audio/timer (vanilla Keen kind-of has these mixed) ***/
void BE_SDL_StartAudioSDService(void (*funcPtr)(void));
void BE_SDL_StopAudioSDService(void);
void BE_SDL_LockAudioRecursively(void);
void BE_SDL_UnlockAudioRecursively(void);
// Frequency is about 1193182Hz/spkVal
void BE_SDL_PCSpeakerOn(id0_word_t spkVal);
void BE_SDL_PCSpeakerOff(void);
// Drop-in replacement for id_sd.c:alOut
void BE_SDL_ALOut(id0_byte_t reg,id0_byte_t val);
// Here, the actual rate is about 1193182Hz/speed
// NOTE: isALMusicOn is irrelevant for Keen Dreams (even with its music code)
void BE_SDL_SetTimer(id0_word_t speed, bool isALMusicOn);
id0_longword_t BE_SDL_GetTimeCount(void);
void BE_SDL_SetTimeCount(id0_longword_t newcount);

/*** Graphics ***/
void BE_SDL_InitGfx(void);
void BE_SDL_ShutdownGfx(void);
void BE_SDL_SetScreenStartAddress(id0_unsigned_t crtc);
id0_byte_t *BE_SDL_GetTextModeMemoryPtr(void);
id0_byte_t *BE_SDL_GetCGAMemoryPtr(void);
id0_byte_t *BE_SDL_GetEGAMemoryPtr(void);
void BE_SDL_SetBorderColor(id0_byte_t color);
void BE_SDL_SetScreenMode(int mode);
void BE_SDL_WaitVBL(id0_int_t number);
void BE_SDL_ShortSleep(void);
