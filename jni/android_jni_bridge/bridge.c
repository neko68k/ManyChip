#include "bridge.h"

#include <jni.h>
#include <android/log.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// lib includes

#include "../gme/gme/gme.h"
#include "../sexypsf/driver.h"
#include "../sexypsf/PsxCommon.h"

#include "../highlyadvanced/VBA/psftag.h"
#include "../highlyadvanced/gsf.h"

#include "aosdk.h"

#define JNIAPI(a,b) JNIEXPORT a JNICALL Java_com_neko68k_ManyChip_NDKBridge_##b

#ifdef __cplusplus
extern "C" {
#endif

#define BUFSIZE 1024


// options shit
static uint32_t mixrate = 44100;
static uint32_t decoder = 0;
static bool inited = false;

// emulator structs
static Music_Emu *gme_emu = NULL;
static PSFINFO *pi;

//HA stuff
int defvolume=1000;
int relvolume=1000;
int TrackLength=0;
int FadeLength=0;
int IgnoreTrackLength, DefaultLength=150000;
int playforever=0;
int fileoutput=0;
int TrailingSilence=1000;
int DetectSilence=0, silencedetected=0, silencelength=5;


int cpupercent=0, sndSamplesPerSec, sndNumChannels;
int sndBitsPerSample=16;

int deflen=120,deffade=4;

extern unsigned short soundFinalWave[1470];
extern int soundBufferLen;

extern char soundEcho;
extern char soundLowPass;
extern char soundReverse;
extern char soundQuality;

double decode_pos_ms; // current decoding position, in milliseconds
int seek_needed; // if != -1, it is the point that the decode thread should seek to, in ms.

static int g_playing = 0;
static int g_must_exit = 0;

jbyteArray jb;

enum Decoders{
	GME = 1,
	SID,
	DUMB,
	HA,
	MOD,
	PSF,
	GSF,
	MDX,
	AO
};

jclass NDKBridge;
jmethodID callbackPA_Write;
JNIEnv *cbEnv;
JavaVM *VM;


jint JNI_OnLoad(JavaVM* vm, void* reserved){
	
	JNIEnv *env;
	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK)
	return -1;
	VM = vm;

	cbEnv = env;
	NDKBridge = (*env)->FindClass(env, "com/neko68k/ManyChip/NDKBridge");
	
	callbackPA_Write = (*env)->GetStaticMethodID(env, NDKBridge, "PA_Write", "([BI)V");
	
	return JNI_VERSION_1_4;
}

JNIAPI(void, StopEngine)(JNIEnv* env, jobject thiz){
	inited = false;
	decoder = 0;
	switch(decoder){
	case GSF:
		//GSFClose();
		break;
	case GME:
		gme_delete(gme_emu);
		gme_emu = NULL;
	default:
		return;
	}
}

JNIAPI(void, Load)(JNIEnv* env, jobject thiz, jstring fn){
	// first we check the extension of the file and decide what lib we're going to
	// load this file with
	// then we load the file
	// this should also handle init/deinit of the appropriate lib

	inited = false;

	char *str;
	str = (char*)(*env)->GetStringUTFChars(env, fn, NULL);
	char *ext;
	ext = strrchr(str, '.');

	if(!strcasecmp(ext, ".vgm") || !strcasecmp(ext, ".gbs") || !strcasecmp(ext, ".ay")\
	|| !strcasecmp(ext, ".gym") || !strcasecmp(ext, ".hes") || !strcasecmp(ext, ".kss")\
	|| !strcasecmp(ext, ".nsf") || !strcasecmp(ext, ".sap") || !strcasecmp(ext, ".spc")\
	|| !strcasecmp(ext, ".vgz") || !strcasecmp(ext, ".nsfe")){
		decoder = GME;
		// will eventually convert this to pop a toast on error
		__android_log_print(ANDROID_LOG_INFO, "ManyChip", "Error: %s", gme_open_file(str, &gme_emu, mixrate));
		if(gme_emu)
			inited = true;
	}
	/*else if(!strcasecmp(ext, ".psf") || !strcasecmp(ext, ".minipsf")){
		decoder = PSF;
		if(pi = sexy_load(str)){
			inited = true;
		}
	}*/
	else if(!strcasecmp(ext, ".gsf") || !strcasecmp(ext, ".minigsf")){
		//StopEngine();
		decoder = GSF;
		inited = GSFRun(str);
		decode_pos_ms = 0;
		seek_needed = -1;
		TrailingSilence=1000;
		TrackLength = DefaultLength;
	}
	else if(!strcasecmp(ext, ".psf") || !strcasecmp(ext, ".minipsf") ||		
		!strcasecmp(ext, ".ssf") || !strcasecmp(ext, ".minissf") ||
		!strcasecmp(ext, ".dsf") || !strcasecmp(ext, ".minidsf") ||
		!strcasecmp(ext, ".psf2") || !strcasecmp(ext, ".minipsf2"))
		{
		decoder = AO;
		__android_log_print(ANDROID_LOG_INFO, "ManyChip", "File: %s", str);
		ao_run(str);
		inited = true;
	}
	(*env)->ReleaseStringUTFChars(env, fn, str);

}



JNIAPI(jbyteArray, GenerationCallback)(JNIEnv* env, jobject thiz){
	// check the global for what lib we are using
	// switch/case and call the proper update
	// func
	short int buffer[BUFSIZE];
	//short int buffer2[44100*2];
	jbyteArray jb;
	JNIEnv *Env = NULL;
	(*VM)->AttachCurrentThread(VM, (JNIEnv **)&Env, NULL);
	//__android_log_print(ANDROID_LOG_INFO, "ManyChip", "Generating");
	if(inited){
		switch(decoder){
		case GME:
			gme_play(gme_emu, BUFSIZE, buffer);
			jb=(*env)->NewByteArray(env, BUFSIZE*2);
			(*env)->SetByteArrayRegion(env, jb, 0,
					BUFSIZE*2, (jbyte *)buffer);
			return(jb);
		break;
		case AO:
			ao_do_frame(BUFSIZE, buffer);
			jb=(*env)->NewByteArray(env, BUFSIZE*2);
			(*env)->SetByteArrayRegion(env, jb, 0,
					BUFSIZE*2, (jbyte *)buffer);
			return(jb);
		break;
		}
	}

}
JNIAPI(void, Play)(JNIEnv* env, jobject thiz, int32_t track){
	if(inited){
	//__android_log_print(ANDROID_LOG_INFO, "ManyChip", "Playing");
		switch(decoder){
		case GME:
			//gme_set_tempo( gme_emu, 0.5 );
			gme_start_track( gme_emu, track -1 );
		break;
		/*case PSF:
			jb=(*env)->NewByteArray(env, 44100);
			sexy_execute();
			
		break;*/
		case GSF:
			
			EmulationLoop();
			
		break;
		}
	}
}

JNIAPI(jint, GetDecoder)(JNIEnv* env, jobject thiz){
	return decoder;
}

/*JNIAPI(void, PSF_Execute)(JNIEnv* env, jobject thiz){

	sexy_execute();
}*/

/*JNIAPI(void, Init)(uint32_t rate, uint8_t bits, bool stereo){
	// bits defaults to 16 if you put 0
}*/

/*JNIAPI(void, SID_Init)()

}*/

// sexypsf stuff
/*void sexyd_update(unsigned char* pSound, long lBytes){
	JNIEnv *Env = NULL;
	(*VM)->AttachCurrentThread(VM, (JNIEnv **)&Env, NULL);
//	jbyteArray jb;
	
	unsigned char *test = (unsigned char *)(*Env)->GetPrimitiveArrayCritical(Env, jb,0);
//	(*Env)->SetByteArrayRegion(Env, jb, 0,
			//lBytes, pSound);
	memcpy(test, pSound, lBytes);	// might have to do this instead
	
	(*Env)->CallStaticVoidMethod(Env, NDKBridge, callbackPA_Write, jb, lBytes);
	//(*cbEnv)->ReleaseByteArrayElements(cbEnv, jb, pSound, 0);
	(*Env)->ReleasePrimitiveArrayCritical(Env, jb,pSound,JNI_ABORT);
	//(*VM)->DetachCurrentThread(VM);
}
*/
// HA stuff
void end_of_track()
{
	g_playing = 0;
}

void writeSound(void)
{
	// TODO: update the sound push/pull funcs to use GetPrimitiveArrayCritical
	// if this speeds things up I'll do it to M1 also and push an update
	
	int tmp;
	int ret = soundBufferLen;
	JNIEnv *Env = NULL;
	(*VM)->AttachCurrentThread(VM, (JNIEnv **)&Env, NULL);
	

	jbyteArray jb;
	jb=(*Env)->NewByteArray(Env, soundBufferLen);
	//(*Env)->SetByteArrayRegion(Env, jb, 0,
	//		soundBufferLen, (jbyte *)soundFinalWave);
	unsigned short *test = (unsigned short *)(*Env)->GetPrimitiveArrayCritical(Env, jb,0);
	//test = soundFinalWave;	
	memcpy(test, soundFinalWave, soundBufferLen);	// might have to do this instead
	(*Env)->ReleasePrimitiveArrayCritical(Env, jb,test,JNI_ABORT);
	(*Env)->CallStaticVoidMethod(Env, NDKBridge, callbackPA_Write, jb, soundBufferLen);
	//(*cbEnv)->ReleaseByteArrayElements(cbEnv, jb, (jbyte*)soundFinalWave, 0);
	//(*VM)->DetachCurrentThread(VM);
	decode_pos_ms += (ret/(2*sndNumChannels) * 1000)/(float)sndSamplesPerSec;
}

#ifdef __cplusplus
}
#endif

