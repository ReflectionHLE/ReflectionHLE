LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SRC := $(LOCAL_PATH)/../..
BESRC := $(LOCAL_PATH)/../../..

SDL_PATH := $(BESRC)/android-lib/jni/SDL

LOCAL_CFLAGS += -std=c99

LOCAL_C_INCLUDES := $(SDL_PATH)/include $(SRC) $(BESRC)

LOCAL_CFLAGS += -DREFKEEN_VER_KDREAMS=1 -DREFKEEN_RESAMPLER_NONE -DREFKEEN_ENABLE_LAUNCHER

LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	$(wildcard $(BESRC)/*.c) \
	$(wildcard $(BESRC)/crc32/*.c) \
	$(wildcard $(BESRC)/opl/*.c) \
	$(wildcard $(BESRC)/unlzexe/*.c) \
	$(filter-out $(wildcard $(SRC)/id_us_s*.c), $(wildcard $(SRC)/*.c)) \
	$(wildcard $(SRC)/lscr/*.c)

LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)

LOCAL_LDLIBS := $(BESRC)/android-lib/libs/$(TARGET_ARCH_ABI)/libSDL2.so -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
