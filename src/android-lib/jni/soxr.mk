#libsoxr

LIB_VERSION:=soxr

LOCAL_PATH:= $(call my-dir)
LIB_ROOT_REL:= $(LIB_VERSION)
LIB_ROOT_ABS:= $(LOCAL_PATH)/$(LIB_VERSION)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -Wall -DSOXR_LIB -DPFFFT_SIMD_DISABLE

LOCAL_SRC_FILES := \
 $(LIB_ROOT_REL)/src/data-io.c \
 $(LIB_ROOT_REL)/src/dbesi0.c \
 $(LIB_ROOT_REL)/src/fft4g32.c \
 $(LIB_ROOT_REL)/src/fft4g64.c \
 $(LIB_ROOT_REL)/src/filter.c \
 $(LIB_ROOT_REL)/src/lsr.c \
 $(LIB_ROOT_REL)/src/pffft.c \
 $(LIB_ROOT_REL)/src/rate32.c \
 $(LIB_ROOT_REL)/src/rate64.c \
 $(LIB_ROOT_REL)/src/soxr.c \
 $(LIB_ROOT_REL)/src/vr32.c

LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/config_soxr \
 $(LIB_ROOT_ABS)/src

LOCAL_LDLIBS := 

LOCAL_MODULE := libsoxr

include $(BUILD_SHARED_LIBRARY)
