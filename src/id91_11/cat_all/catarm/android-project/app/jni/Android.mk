LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)

SRC := $(LOCAL_PATH)/../..
BESRC := $(LOCAL_PATH)/../../../../..
C3DCOMMONSRC := $(LOCAL_PATH)/../../..
CATLATECOMMONSRC := $(LOCAL_PATH)/../../../cat_late
IDSRC := $(LOCAL_PATH)/../../../..
EXTSRC := $(LOCAL_PATH)/../../../external_restored

LOCAL_C_INCLUDES := $(C3DCOMMONSRC) $(CATLATECOMMONSRC) $(IDSRC) $(EXTSRC)

LOCAL_CFLAGS += -DREFKEEN_VER_CATADVENTURES=1 -DREFKEEN_VER_CATARM=1 -DREFKEEN_VER_CATACOMB_ALL=1

LOCAL_SRC_FILES := \
	$(wildcard $(BESRC)/*.c) \
	$(wildcard $(BESRC)/crc32/*.c) \
	$(wildcard $(BESRC)/opl/*.c) \
	$(wildcard $(BESRC)/unlzexe/*.c) \
	$(wildcard $(EXTSRC)/*.c) \
	$(filter-out $(IDSRC)/id_us_2.c, $(wildcard $(IDSRC)/*.c)) \
	$(wildcard $(CATLATECOMMONSRC)/*.c) \
	$(wildcard $(C3DCOMMONSRC)/*.c) \
	$(wildcard $(SRC)/*.c)

include $(LOCAL_PATH)/../../../../../android-lib/app/jni/Android_common.mk
