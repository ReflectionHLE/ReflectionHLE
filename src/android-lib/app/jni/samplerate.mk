#libsamplerate

LIB_VERSION:=samplerate

LOCAL_PATH:= $(call my-dir)
LIB_ROOT_REL:= $(LIB_VERSION)
LIB_ROOT_ABS:= $(LOCAL_PATH)/$(LIB_VERSION)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -DHAVE_CONFIG_H -Werror -g

LOCAL_SRC_FILES := \
 $(LIB_ROOT_REL)/src/samplerate.c \
 $(LIB_ROOT_REL)/src/src_linear.c \
 $(LIB_ROOT_REL)/src/src_sinc.c \
 $(LIB_ROOT_REL)/src/src_zoh.c


LOCAL_C_INCLUDES := \
 $(LOCAL_PATH)/config_samplerate \
 $(LIB_ROOT_ABS)/src

LOCAL_LDLIBS := -llog -lm

LOCAL_MODULE := libsamplerate

include $(BUILD_SHARED_LIBRARY)
