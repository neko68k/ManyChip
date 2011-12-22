#include "jniwrap.h"
#include "mxdrvg/mxdrvg.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#define LOG_D(msg) __android_log_write(ANDROID_LOG_DEBUG, "jniwrap.cpp", msg)
#else
#define LOG_D(msg) 
#endif

// Global env ref (for callbacks)
static JavaVM *g_VM;

jclass jNativesCls;

static jmethodID onOPMIntFunc = NULL;
static jmethodID onTerminatePlayFunc = NULL;

static MXDRVG_WORK_CH *FM;
static MXDRVG_WORK_CH *PCM;
static MXDRVG_WORK_GLOBAL *G;
static MXDRVG_WORK_OPM *OPM;
static int UseIRQ;

static int Terminated;
static int FadeoutStart;
static int FadeoutRequest;
static int LoopLimit;

jint JNI_OnLoad(
	JavaVM *vm,
	void *reserved
) {
	LOG_D("JNI_OnLoad()");
	return JNI_VERSION_1_6;
}

void JNI_OnUnload(
	JavaVM *vm,
	void *reserved
) {
	LOG_D("JNI_OnUnload()");
}


void jniwrap_MXDRVG_OPMINTFUNC(
	void
);

//JNIEXPORT jint JNICALL Java_net_gorry_ndk_Natives_ndkEntry
JNIAPI(jint, ndkEntry)(
	JNIEnv *env,
	jclass cls,
	jobjectArray jargv
) {
	LOG_D("ndkEntry()");
	return 0;
}

JNIAPI(jint, mxdrvgStart)(
	JNIEnv *env,
	jclass cls, 
	jint samprate,
	jint fastmode,
	jint mdxbufsize,
	jint pdxbufsize,
	jint useirq
) {
	int size;
	int ret;

	LOG_D("MXDRVG_Start()");

	if (!env) {
		LOG_D("MXDRV_Start(): error !env");
		return -1;
	}

	env->GetJavaVM(&g_VM);
	// (*env)->GetJavaVM(env, &g_VM);
	if (!g_VM) {
		LOG_D("MXDRV_Start(): error !g_VM");
		return -1;
	}

#if 0
	jNativesCls = env->FindClass("Ljava/lang/Object;");
	if (!jNativesCls) {
		LOG_D("MXDRV_Start(): error !jNativesCls");
	}
#endif
	jNativesCls = cls;

	UseIRQ = useirq;
	ret = MXDRVG_Start(samprate, fastmode, mdxbufsize, pdxbufsize);

	FM = (MXDRVG_WORK_CH *)MXDRVG_GetWork( MXDRVG_WORKADR_FM );
	PCM = (MXDRVG_WORK_CH *)MXDRVG_GetWork( MXDRVG_WORKADR_PCM );
	G = (MXDRVG_WORK_GLOBAL *)MXDRVG_GetWork( MXDRVG_WORKADR_GLOBAL );
	OPM = (MXDRVG_WORK_OPM *)MXDRVG_GetWork( MXDRVG_WORKADR_OPM );

	MXDRVG_CALLBACK_OPMINTFUNC **p;
	p = (MXDRVG_CALLBACK_OPMINTFUNC **)MXDRVG_GetWork( MXDRVG_WORKADR_OPMINT );
	*(p) = &jniwrap_MXDRVG_OPMINTFUNC;

	return ret;
}

JNIAPI(void, mxdrvgEnd)(
	JNIEnv *env,
	jclass cls,
	int dummy
) {
	LOG_D("MXDRVG_End()");
	MXDRVG_End();
}

JNIAPI(jint, mxdrvgGetPCM)(
	JNIEnv *env,
	jclass cls, 
	jarray buf,
	jint ofs,
	jint len
) {
//	LOG_D("MXDRVG_GetPCM()");
	jshort *a = (jshort *)env->GetPrimitiveArrayCritical(buf, 0);
	MXDRVG_GetPCM(a+ofs*2, len);
#if 0
	{
		int i;
		for (i=0; i<len; i++) {
			if (a[len*2+0] || a[len*2+1]) {
				LOG_D("sound found");
			}
		}
	}
#endif
	env->ReleasePrimitiveArrayCritical(buf, a, 0);
	if (Terminated) {
		LOG_D("MXDRVG_GetPCM(): Terminated");
	} else {
		LOG_D("MXDRVG_GetPCM(): not Terminated");
	}
	return Terminated;
}

JNIAPI(void, mxdrvgSetData)(
	JNIEnv *env,
	jclass cls, 
	jarray mdx,
	jint mdxsize,
	jarray pdx,
	jint pdxsize
) {
	LOG_D("MXDRVG_SetData()");
	if ((mdx == NULL) || (mdxsize == 0)) {
		return;
	}
	jbyte *jmdx = (jbyte *)env->GetPrimitiveArrayCritical(mdx, 0);
	jbyte *jpdx = NULL;
	if ((pdx != NULL) && (pdxsize != 0)) {
		jpdx = (jbyte *)env->GetPrimitiveArrayCritical(pdx, 0);
	}
#if 0
	if (jpdx) {
		int i;
		for (i=0; i<256; i+=16) {
			char s[1024];
			unsigned char *p = (unsigned char *)jpdx;
			sprintf(s, "%02X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", i,
			p[i+0x00], p[i+0x01], p[i+0x02], p[i+0x03], 
			p[i+0x04], p[i+0x05], p[i+0x06], p[i+0x07], 
			p[i+0x08], p[i+0x09], p[i+0x0a], p[i+0x0b], 
			p[i+0x0c], p[i+0x0d], p[i+0x0e], p[i+0x0f]
			);
			LOG_D(s);
		}
	}
#endif
	MXDRVG_SetData(jmdx, mdxsize, jpdx, pdxsize);
	if (jpdx) {
		env->ReleasePrimitiveArrayCritical(pdx, jpdx, 0);
	}
	env->ReleasePrimitiveArrayCritical(mdx, jmdx, 0);
	Terminated = 0;
}

JNIAPI(jint, mxdrvgMeasurePlayTime)(
	JNIEnv *env,
	jclass cls, 
	jint loop,
	jint fadeout
) {
	LOG_D("MXDRVG_MeasurePlayTime()");
	jint ret = MXDRVG_MeasurePlayTime(loop, fadeout);
	return ret;
}

JNIAPI(void, mxdrvgPlayAt)(
	JNIEnv *env,
	jclass cls, 
	jint playat,
	jint loop,
	jint fadeout
) {
	LOG_D("MXDRVG_PlayAt()");
	LoopLimit = loop;
	FadeoutRequest = fadeout;
	FadeoutStart = 0;
	Terminated = 0;
	MXDRVG_PlayAt(playat, loop, fadeout);
}

JNIAPI(jint, mxdrvgGetPlayAt)(
	JNIEnv *env,
	jclass cls, 
	jint dummy
) {
	LOG_D("MXDRVG_GetPlayAt()");
	return MXDRVG_GetPlayAt();
}

JNIAPI(jint, mxdrvgGetTerminated)(
	JNIEnv *env,
	jclass cls, 
	jint dummy
) {
	LOG_D("MXDRVG_GetTerminated()");
	return Terminated;
}

JNIAPI(void, mxdrvgTotalVolume)(
	JNIEnv *env,
	jclass cls, 
	jint vol
) {
	LOG_D("MXDRVG_TotalVolume()");
	MXDRVG_TotalVolume(vol);
}

JNIAPI(jint, mxdrvgGetTotalVolume)(
	JNIEnv *env,
	jclass cls, 
	jint vol
) {
	LOG_D("MXDRVG_GetTotalVolume()");
	return MXDRVG_GetTotalVolume();
}

JNIAPI(void, mxdrvgChannelMask)(
	JNIEnv *env,
	jclass cls, 
	jint mask
) {
	LOG_D("MXDRVG_ChannelMask()");
	MXDRVG_ChannelMask(mask);
}

JNIAPI(jint, mxdrvgGetChannelMask)(
	JNIEnv *env,
	jclass cls, 
	jint dummy
) {
	LOG_D("MXDRVG_GetChannelMask()");
	return MXDRVG_GetChannelMask();
}

JNIAPI(void, mxdrvgFadeout)(
	JNIEnv *env,
	jclass cls,
	jint dummy
) {
	LOG_D("MXDRVG_Fadeout()");
	MXDRVG_Fadeout();
}

JNIAPI(void, mxdrvgFadeout2)(
	JNIEnv *env,
	jclass cls,
	jint a
) {
	LOG_D("MXDRVG_Fadeout2()");
	MXDRVG_Fadeout2(a)
}

void jniwrap_MXDRVG_OPMINTFUNC(
	void
) {
	// LOG_D("MXDRVG_OPMINTFUNC()");

	JNIEnv *env = NULL;
	jint result;

	if (!g_VM) {
		// ERROR0("jni_NdkUpdateView: No JNI VM available.\n");
	} else if (!jNativesCls) {
		// ERROR0("jni_NdkUpdateView: No JNI jNativesCls available.\n");
	} else {
		result = g_VM->AttachCurrentThread((JNIEnv **)&env, NULL);
		if (result != JNI_OK)
		{
			// ERROR0("jni_NdkUpdateView: AttachCurrentThread() failed.\n");
		} else if (!env) {
			// ERROR0("jni_NdkUpdateView: AttachCurrentThread() failed.\n");
		}
	}

	if (UseIRQ & 2) {
		if (env) {
			if (!onOPMIntFunc) {
				// Natives.class#onOPMIntFunc -> javap -s Natives
				onOPMIntFunc = env->GetStaticMethodID(
				  jNativesCls,
				  "onOPMIntFunc",
				  "(II[B[B)V"
				);
			}

			if (onOPMIntFunc) {
				jbyteArray jMdxChannelWork = NULL;
				jbyteArray jMdxGlobalWork = NULL;
				int size;

				size = sizeof(MXDRVG_WORK_CH)*16;
				jMdxChannelWork = env->NewByteArray(size);

				size = sizeof(MXDRVG_WORK_GLOBAL);
				jMdxGlobalWork = env->NewByteArray(size);

				int i;
				for (i=0; i<9; i++) {
					env->SetByteArrayRegion(
					  jMdxChannelWork, i*sizeof(MXDRVG_WORK_CH), sizeof(MXDRVG_WORK_CH), 
					  (jbyte *)(&FM[i])
					);
				}
				for (i=0; i<7; i++) {
					env->SetByteArrayRegion(
					  jMdxChannelWork, (i+9)*sizeof(MXDRVG_WORK_CH), sizeof(MXDRVG_WORK_CH), 
					  (jbyte *)(&PCM[i])
					);
				}

				env->SetByteArrayRegion(
				  jMdxGlobalWork, 0, sizeof(MXDRVG_WORK_GLOBAL), 
				  (jbyte *)G
				);

				env->CallStaticVoidMethod(
				  jNativesCls,
				  onOPMIntFunc,
				  16,
				  sizeof(MXDRVG_WORK_CH),
				  jMdxChannelWork,
				  jMdxGlobalWork
				);

				env->DeleteLocalRef(jMdxChannelWork);
				env->DeleteLocalRef(jMdxGlobalWork);
			}
		}
	}

	if (!Terminated) {
		if (G->L001e13 != 0) {
			 LOG_D("MXDRVG_OPMINTFUNC(): Terminated by L001e13");
			Terminated = 1;
		}
		if (G->L002246 == 65535) {
			 LOG_D("MXDRVG_OPMINTFUNC(): Terminated by L002246");
			Terminated = 1;
		} else {
			int loopcount;
			loopcount = G->L002246;
			if ( !FadeoutStart ) {
				if ( loopcount >= LoopLimit ) {
					if ( FadeoutRequest ) {
						FadeoutStart = TRUE;
						MXDRVG_Fadeout();
					} else {
						 LOG_D("MXDRVG_OPMINTFUNC(): Terminated by LOOPCOUNT");
						Terminated = 1;
					}
				}
			}
		}
	}
	if (UseIRQ & 1) {
		if (Terminated) {
			JNIEnv *env;
			jint result;
			if (env) {
				if (!onTerminatePlayFunc) {
					// Natives.class#onTerminatePlayFunc -> javap -s Natives
					onTerminatePlayFunc = env->GetStaticMethodID(
					  jNativesCls,
					  "onTerminatePlayFunc",
					  "(I)V"
					);
				}
				if (onTerminatePlayFunc) {
					env->CallStaticVoidMethod(
					  jNativesCls,
					  onTerminatePlayFunc,
					  0
					);
				}
			}
		}
	}
}

#ifdef __cplusplus
}
#endif // __cplusplus

// [EOF]
