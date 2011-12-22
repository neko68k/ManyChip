LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sexypsf
LOCAL_LDLIBS := -lz
LOCAL_CFLAGS := -O3

LOCAL_SRC_FILES := Misc.c  PsxBios.c  PsxCounters.c  PsxDma.c  PsxHLE.c  PsxHw.c  PsxInterpreter.c  PsxMem.c  R3000A.c  Spu.c
LOCAL_SRC_FILES += spu/spu.c 
#spu/dma.c spu/registers.c spu/reverb.c spu/adsr.c


include $(BUILD_STATIC_LIBRARY)