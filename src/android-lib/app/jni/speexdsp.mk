#libspeexdsp

LIB_VERSION:=speexdsp

LOCAL_PATH:= $(call my-dir)
LIB_ROOT_REL:= $(LIB_VERSION)
LIB_ROOT_ABS:= $(LOCAL_PATH)/$(LIB_VERSION)
 
include $(CLEAR_VARS)
 
LOCAL_CFLAGS := -DFIXED_POINT -DUSE_KISS_FFT -DEXPORT="" -DHAVE_STDINT_H -UHAVE_CONFIG_H 

LOCAL_SRC_FILES := \
 $(LIB_ROOT_REL)/libspeexdsp/buffer.c \
 $(LIB_ROOT_REL)/libspeexdsp/fftwrap.c \
 $(LIB_ROOT_REL)/libspeexdsp/filterbank.c \
 $(LIB_ROOT_REL)/libspeexdsp/jitter.c \
 $(LIB_ROOT_REL)/libspeexdsp/kiss_fft.c \
 $(LIB_ROOT_REL)/libspeexdsp/kiss_fftr.c \
 $(LIB_ROOT_REL)/libspeexdsp/mdf.c \
 $(LIB_ROOT_REL)/libspeexdsp/preprocess.c \
 $(LIB_ROOT_REL)/libspeexdsp/resample.c \
 $(LIB_ROOT_REL)/libspeexdsp/scal.c \
 $(LIB_ROOT_REL)/libspeexdsp/smallft.c

LOCAL_C_INCLUDES := \
 $(LOCAL_PATH)/config_speexdsp \
 $(LOCAL_PATH)/config_speexdsp/speex \
 $(LIB_ROOT_ABS)/include \
 $(LIB_ROOT_ABS)/src

LOCAL_LDLIBS := -llog

LOCAL_MODULE := libspeexdsp

include $(BUILD_SHARED_LIBRARY)
