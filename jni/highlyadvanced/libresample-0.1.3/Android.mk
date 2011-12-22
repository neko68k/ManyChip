include $(CLEAR_VARS)
LOCAL_PATH := $(call my-dir)

LOCAL_MODULE := libresample

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := src/resample.c src/resamplesubs.c src/filterkit.c

include $(BUILD_STATIC_LIBRARY)