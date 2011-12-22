LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gme
LOCAL_CFLAGS += -O3
LOCAL_CXXFLAGS := -O2
#LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/gme/gme
LOCAL_SRC_FILES := gme/Ay_Apu.cpp       gme/Dual_Resampler.cpp  gme/Gbs_Emu.cpp   gme/Hes_Emu.cpp       gme/Music_Emu.cpp      gme/Nes_Vrc6_Apu.cpp  gme/Sms_Apu.cpp     gme/Vgm_Emu.cpp
LOCAL_SRC_FILES += gme/Ay_Cpu.cpp       gme/Effects_Buffer.cpp  gme/gme.cpp       gme/Kss_Cpu.cpp       gme/Nes_Apu.cpp        gme/Nsfe_Emu.cpp      gme/Snes_Spc.cpp    gme/Vgm_Emu_Impl.cpp
LOCAL_SRC_FILES += gme/Ay_Emu.cpp       gme/Fir_Resampler.cpp   gme/Gme_File.cpp  gme/Kss_Emu.cpp       gme/Nes_Cpu.cpp        gme/Nsf_Emu.cpp       gme/Spc_Cpu.cpp     gme/Ym2413_Emu.cpp
LOCAL_SRC_FILES += gme/Blip_Buffer.cpp  gme/Gb_Apu.cpp          gme/Gym_Emu.cpp   gme/Kss_Scc_Apu.cpp   gme/Nes_Fme7_Apu.cpp   gme/Sap_Apu.cpp       gme/Spc_Dsp.cpp     gme/Ym2612_Emu.cpp
LOCAL_SRC_FILES += gme/Classic_Emu.cpp  gme/Gb_Cpu.cpp          gme/Hes_Apu.cpp   gme/M3u_Playlist.cpp  gme/Nes_Namco_Apu.cpp  gme/Sap_Cpu.cpp       gme/Spc_Emu.cpp
LOCAL_SRC_FILES += gme/Data_Reader.cpp  gme/Gb_Oscs.cpp         gme/Hes_Cpu.cpp   gme/Multi_Buffer.cpp  gme/Nes_Oscs.cpp       gme/Sap_Emu.cpp       gme/Spc_Filter.cpp

#LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

include $(BUILD_STATIC_LIBRARY)