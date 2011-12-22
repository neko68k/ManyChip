package com.neko68k.ManyChip;


public class NDKBridge {	
	
	static PlayerService playerService = null;
	static PushAudio pa = null;
	
	
	public static native byte[] GenerationCallback();
	public static native void Stop();
	public static native void Play(int track);
	public static native void Load(String fn);
	public static native int GetDecoder();
	public static native void StopEngine();
	
	// callbacks
	public static void PA_Write(byte buffer[], int bytes){
		pa.write(buffer, bytes);
	}
	
	static {
        System.loadLibrary("manychip");
    }
}
