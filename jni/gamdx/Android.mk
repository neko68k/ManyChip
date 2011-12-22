LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := mxdrvg
LOCAL_SRC_FILES := downsample/downsample.cpp    fmgen/fmgen.cpp fmgen/fmtimer.cpp fmgen/opm.cpp    mxdrvg/so.cpp    pcm8/pcm8.cpp pcm8/x68pcm8.cpp



include $(BUILD_STATIC_LIBRARY)
