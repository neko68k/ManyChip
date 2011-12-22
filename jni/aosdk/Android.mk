LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := aosdk
LOCAL_LDLIBS := -lz
LOCAL_CFLAGS := -O3 -DLSB_FIRST=1 -c -O3 -DPATH_MAX=1024 -DHAS_PSXCPU=1 -I. -I.. -Ieng_ssf -Ieng_qsf  -Ieng_dsf 

# main objects
LOCAL_SRC_FILES += corlett.c


# DSF engine
LOCAL_SRC_FILES += eng_dsf/eng_dsf.c eng_dsf/dc_hw.c eng_dsf/aica.c eng_dsf/aicadsp.c eng_dsf/arm7.c eng_dsf/arm7i.c

# SSF engine
LOCAL_SRC_FILES += eng_ssf/m68kcpu.c eng_ssf/m68kopac.c eng_ssf/m68kopdm.c eng_ssf/m68kopnz.c eng_ssf/m68kops.c 
LOCAL_SRC_FILES += eng_ssf/scsp.c eng_ssf/scspdsp.c eng_ssf/sat_hw.c eng_ssf/eng_ssf.c

# QSF engine
LOCAL_SRC_FILES += eng_qsf/eng_qsf.c eng_qsf/kabuki.c eng_qsf/qsound.c eng_qsf/z80.c eng_qsf/z80dasm.c

# PSF engine
LOCAL_SRC_FILES += eng_psf/eng_psf.c eng_psf/psx.c eng_psf/psx_hw.c eng_psf/peops/spu.c 

# PSF2 extentions
LOCAL_SRC_FILES += eng_psf/eng_psf2.c eng_psf/peops2/spu.c eng_psf/peops2/dma.c eng_psf/peops2/registers.c

# SPU engine (requires PSF engine)
LOCAL_SRC_FILES += eng_psf/eng_spu.c

# zlib (included for max portability)
#LOCAL_SRC_FILES += zlib/adler32.c zlib/compress.c zlib/crc32.c zlib/gzio.c zlib/uncompr.c zlib/deflate.c zlib/trees.c
#LOCAL_SRC_FILES += zlib/zutil.c zlib/inflate.c zlib/infback.c zlib/inftrees.c zlib/inffast.c

#spu/dma.c spu/registers.c spu/reverb.c spu/adsr.c


include $(BUILD_STATIC_LIBRARY)
