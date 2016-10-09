LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC := $(LOCAL_PATH)/../..
BESRC := $(LOCAL_PATH)/../../../../..
C3DCOMMONSRC := $(LOCAL_PATH)/../../..
IDSRC := $(LOCAL_PATH)/../../../..

LOCAL_C_INCLUDES := $(C3DCOMMONSRC) $(IDSRC)

LOCAL_CFLAGS += -DREFKEEN_VER_CAT3D=1 -DREFKEEN_VER_CATACOMB_ALL=1

LOCAL_SRC_FILES := \
	$(wildcard $(BESRC)/*.c) \
	$(wildcard $(BESRC)/crc32/*.c) \
	$(wildcard $(BESRC)/opl/*.c) \
	$(wildcard $(BESRC)/unlzexe/*.c) \
	$(wildcard $(IDSRC)/*.c) \
	$(wildcard $(C3DCOMMONSRC)/*.c) \
	$(wildcard $(SRC)/*.c)

include $(LOCAL_PATH)/../../../../../android-lib/jni/Android_common.mk
