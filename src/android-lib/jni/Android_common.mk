# Included from each game Android.mk file

LOCAL_MODULE := main

SDL_PATH := $(BESRC)/android-lib/jni/SDL

LOCAL_CFLAGS += -std=c99

LOCAL_C_INCLUDES += $(SDL_PATH)/include $(SRC) $(BESRC)

RESAMPLER := LIBSPEEXDSP

LOCAL_CFLAGS += -DREFKEEN_RESAMPLER_$(RESAMPLER) -DREFKEEN_ENABLE_LAUNCHER

LOCAL_SRC_FILES += $(SDL_PATH)/src/main/android/SDL_android_main.c

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)

LOCAL_LDLIBS := $(BESRC)/android-lib/libs/$(TARGET_ARCH_ABI)/libSDL2.so -lGLESv1_CM -lGLESv2 -llog

ifeq ($(RESAMPLER),LIBSOXR)
	LOCAL_C_INCLUDES += $(BESRC)/android-lib/jni/soxr/src $(BESRC)/android-lib/jni/config_samplerate
	LOCAL_LDLIBS += $(BESRC)/android-lib/libs/$(TARGET_ARCH_ABI)/libsoxr.so
else ifeq ($(RESAMPLER),LIBSAMPLERATE)
	LOCAL_C_INCLUDES += $(BESRC)/android-lib/jni/samplerate/src $(BESRC)/android-lib/jni/config_samplerate
	LOCAL_LDLIBS += $(BESRC)/android-lib/libs/$(TARGET_ARCH_ABI)/libsamplerate.so
else ifeq ($(RESAMPLER),LIBSPEEXDSP)
	LOCAL_C_INCLUDES += $(BESRC)/android-lib/jni/speexdsp/include $(BESRC)/android-lib/jni/config_speexdsp
	LOCAL_LDLIBS += $(BESRC)/android-lib/libs/$(TARGET_ARCH_ABI)/libspeexdsp.so
endif

include $(BUILD_SHARED_LIBRARY)
