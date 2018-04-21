# Included from each game Android.mk file

LOCAL_MODULE := main

SDL_PATH := $(BESRC)/android-lib/app/jni/SDL

LOCAL_CFLAGS += -std=c99

LOCAL_C_INCLUDES += $(SDL_PATH)/include $(SRC) $(BESRC)

RESAMPLER := LIBSPEEXDSP

LOCAL_CFLAGS += -DREFKEEN_RESAMPLER_$(RESAMPLER) -DREFKEEN_ENABLE_LAUNCHER

LOCAL_SRC_FILES += $(SDL_PATH)/src/main/android/SDL_android_main.c

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)

# The following addition of the directory is a HACK; Same with references to libraries like SDL2.
LOCAL_LDLIBS := -L$(BESRC)/android-lib/app/build/intermediates/bundles/$(APP_OPTIM)/jni/$(TARGET_ARCH_ABI) -lSDL2 -lGLESv1_CM -lGLESv2 -llog

ifeq ($(RESAMPLER),LIBSOXR)
	LOCAL_C_INCLUDES += $(BESRC)/android-lib/app/jni/soxr/src $(BESRC)/android-lib/app/jni/config_samplerate
	LOCAL_LDLIBS += -lsoxr
else ifeq ($(RESAMPLER),LIBSAMPLERATE)
	LOCAL_C_INCLUDES += $(BESRC)/android-lib/app/jni/samplerate/src $(BESRC)/android-lib/app/jni/config_samplerate
	LOCAL_LDLIBS += -lsamplerate
else ifeq ($(RESAMPLER),LIBSPEEXDSP)
	LOCAL_C_INCLUDES += $(BESRC)/android-lib/app/jni/speexdsp/include $(BESRC)/android-lib/app/jni/config_speexdsp
	LOCAL_LDLIBS += -lspeexdsp
endif

include $(BUILD_SHARED_LIBRARY)
