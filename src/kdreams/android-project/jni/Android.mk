LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC := $(LOCAL_PATH)/../..
BESRC := $(LOCAL_PATH)/../../..

LOCAL_CFLAGS += -DREFKEEN_VER_KDREAMS=1

LOCAL_SRC_FILES := \
	$(wildcard $(BESRC)/*.c) \
	$(wildcard $(BESRC)/crc32/*.c) \
	$(wildcard $(BESRC)/opl/*.c) \
	$(wildcard $(BESRC)/depklite/*.c) \
	$(wildcard $(BESRC)/unlzexe/*.c) \
	$(filter-out $(wildcard $(SRC)/id_us_s*.c), $(wildcard $(SRC)/*.c)) \
	$(wildcard $(SRC)/lscr/*.c)

include $(LOCAL_PATH)/../../../android-lib/jni/Android_common.mk
