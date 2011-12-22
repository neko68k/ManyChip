LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gme

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/gme/gme
LOCAL_SRC_FILES := gme/gme/Ay_Apu.cpp       gme/gme/Dual_Resampler.cpp  gme/gme/Gbs_Emu.cpp   gme/gme/Hes_Emu.cpp       gme/gme/Music_Emu.cpp      gme/gme/Nes_Vrc6_Apu.cpp  gme/gme/Sms_Apu.cpp     gme/gme/Vgm_Emu.cpp
LOCAL_SRC_FILES += gme/gme/Ay_Cpu.cpp       gme/gme/Effects_Buffer.cpp  gme/gme/gme.cpp       gme/gme/Kss_Cpu.cpp       gme/gme/Nes_Apu.cpp        gme/gme/Nsfe_Emu.cpp      gme/gme/Snes_Spc.cpp    gme/gme/Vgm_Emu_Impl.cpp
LOCAL_SRC_FILES += gme/gme/Ay_Emu.cpp       gme/gme/Fir_Resampler.cpp   gme/gme/Gme_File.cpp  gme/gme/Kss_Emu.cpp       gme/gme/Nes_Cpu.cpp        gme/gme/Nsf_Emu.cpp       gme/gme/Spc_Cpu.cpp     gme/gme/Ym2413_Emu.cpp
LOCAL_SRC_FILES += gme/gme/Blip_Buffer.cpp  gme/gme/Gb_Apu.cpp          gme/gme/Gym_Emu.cpp   gme/gme/Kss_Scc_Apu.cpp   gme/gme/Nes_Fme7_Apu.cpp   gme/gme/Sap_Apu.cpp       gme/gme/Spc_Dsp.cpp     gme/gme/Ym2612_Emu.cpp
LOCAL_SRC_FILES += gme/gme/Classic_Emu.cpp  gme/gme/Gb_Cpu.cpp          gme/gme/Hes_Apu.cpp   gme/gme/M3u_Playlist.cpp  gme/gme/Nes_Namco_Apu.cpp  gme/gme/Sap_Cpu.cpp       gme/gme/Spc_Emu.cpp
LOCAL_SRC_FILES += gme/gme/Data_Reader.cpp  gme/gme/Gb_Oscs.cpp         gme/gme/Hes_Cpu.cpp   gme/gme/Multi_Buffer.cpp  gme/gme/Nes_Oscs.cpp       gme/gme/Sap_Emu.cpp       gme/gme/Spc_Filter.cpp

#LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

include $(BUILD_STATIC_LIBRARY)