LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SRC := $(LOCAL_PATH)/../..
BESRC := $(LOCAL_PATH)/../../../../..
C3DCOMMONSRC := $(LOCAL_PATH)/../../..
CATLATECOMMONSRC := $(LOCAL_PATH)/../../../cat_late
IDSRC := $(LOCAL_PATH)/../../../..
EXTSRC := $(LOCAL_PATH)/../../../external_restored

SDL_PATH := $(BESRC)/android-lib/jni/SDL

LOCAL_CFLAGS += -std=c99

LOCAL_C_INCLUDES := $(SDL_PATH)/include $(SRC) $(BESRC) $(C3DCOMMONSRC) $(CATLATECOMMONSRC) $(IDSRC) $(EXTSRC)

RESAMPLER := LIBSPEEXDSP

LOCAL_CFLAGS += -DREFKEEN_VER_CATADVENTURES=1 -DREFKEEN_VER_CATAPOC=1 -DREFKEEN_VER_CATACOMB_ALL=1 -DREFKEEN_RESAMPLER_$(RESAMPLER) -DREFKEEN_ENABLE_LAUNCHER

LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	$(wildcard $(BESRC)/*.c) \
	$(wildcard $(BESRC)/crc32/*.c) \
	$(wildcard $(BESRC)/opl/*.c) \
	$(wildcard $(BESRC)/unlzexe/*.c) \
	$(wildcard $(EXTSRC)/*.c) \
	$(filter-out $(IDSRC)/id_us_2.c, $(wildcard $(IDSRC)/*.c)) \
	$(wildcard $(CATLATECOMMONSRC)/*.c) \
	$(wildcard $(C3DCOMMONSRC)/*.c) \
	$(wildcard $(SRC)/*.c)

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)

LOCAL_LDLIBS := $(BESRC)/android-lib/libs/$(TARGET_ARCH_ABI)/libSDL2.so -lGLESv1_CM -lGLESv2 -llog

ifeq ($(RESAMPLER),LIBSOXR)
	LOCAL_C_INCLUDES += $(BESRC)/android-lib/jni/soxr/libsoxr/src
	LOCAL_LDLIBS += $(BESRC)/android-lib/libs/$(TARGET_ARCH_ABI)/libsoxr.so
else ifeq ($(RESAMPLER),LIBSAMPLERATE)
	LOCAL_C_INCLUDES += $(BESRC)/android-lib/jni/samplerate/libsamplerate
	LOCAL_LDLIBS += $(BESRC)/android-lib/libs/$(TARGET_ARCH_ABI)/libsamplerate.so
else ifeq ($(RESAMPLER),LIBSPEEXDSP)
	LOCAL_C_INCLUDES += $(BESRC)/android-lib/jni/speexdsp/speexdsp/speexdsp-1.2rc3/include
	LOCAL_LDLIBS += $(BESRC)/android-lib/libs/$(TARGET_ARCH_ABI)/libspeexdsp.so
endif

include $(BUILD_SHARED_LIBRARY)
