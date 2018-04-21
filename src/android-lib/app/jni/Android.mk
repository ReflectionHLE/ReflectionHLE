LOCAL_PATH := $(call my-dir)
EXTRA_MAKEFILES :=

ifneq ($(wildcard $(LOCAL_PATH)/samplerate/*),)
	EXTRA_MAKEFILES += $(LOCAL_PATH)/samplerate.mk
endif

ifneq ($(wildcard $(LOCAL_PATH)/soxr/*),)
	EXTRA_MAKEFILES += $(LOCAL_PATH)/soxr.mk
endif

ifneq ($(wildcard $(LOCAL_PATH)/speexdsp/*),)
	EXTRA_MAKEFILES += $(LOCAL_PATH)/speexdsp.mk
endif

include $(call all-subdir-makefiles) $(EXTRA_MAKEFILES)
