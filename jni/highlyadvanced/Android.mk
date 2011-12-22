LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := highlyadvanced
LOCAL_CFLAGS := -DLINUX -DC_CORE -DNO_INTERPOLATION -O3
LOCAL_CPPFLAGS := -O3
LOCAL_LDLIBS :: -lz
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libresample-0.1.3/include
LOCAL_SRC_FILES := VBA/bios.cpp  VBA/GBA.cpp  VBA/Globals.cpp  VBA/snd_interp.cpp  VBA/Sound.cpp  VBA/unzip.cpp  VBA/Util.cpp VBA/memgzio.c  VBA/psftag.c
LOCAL_SRC_FILES += gsf.cpp
LOCAL_SRC_FILES += libresample-0.1.3/src/resample.c libresample-0.1.3/src/resamplesubs.c libresample-0.1.3/src/filterkit.c

include $(BUILD_STATIC_LIBRARY)

