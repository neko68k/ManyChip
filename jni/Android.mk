JNIPATH := $(call my-dir)
LOCAL_PATH := $(JNIPATH)


include $(call all-subdir-makefiles)

LOCAL_PATH := $(JNIPATH)
include $(CLEAR_VARS)


LOCAL_LDLIBS := -llog -lz
LOCAL_MODULE := manychip

LOCAL_SRC_FILES := android_jni_bridge/bridge.c android_jni_bridge/aosdk.c

LOCAL_STATIC_LIBRARIES := aosdk gme mxdrvg highlyadvanced
include $(BUILD_SHARED_LIBRARY)

