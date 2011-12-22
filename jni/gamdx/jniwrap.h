#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if !defined(__java_net_gorry_mxdrvg_jniwrap_h__)
#define __java_net_gorry_mxdrvg_jniwrap_h__

#include <jni.h>
#include <android/log.h>

#define JNIAPI(a,b) JNIEXPORT a JNICALL Java_net_gorry_ndk_Natives_##b

#ifdef __cplusplus
extern "C" {
#endif

JNIAPI(jint, mxdrvgStart)(
	JNIEnv *env,
	jclass cls, 
	jint samprate,
	jint fastmode,
	jint mdxbufsize,
	jint pdxbufsize,
	jint useirq
);

JNIAPI(void, mxdrvgEnd)(
	JNIEnv *env,
	jclass cls,
	jint dummy
);

JNIAPI(jint, mxdrvgGetPCM)(
	JNIEnv *env,
	jclass cls, 
	jarray buf,
	jint ofs,
	jint len
);

JNIAPI(void, mxdrvgSetData)(
	JNIEnv *env,
	jclass cls, 
	jarray mdx,
	jint mdxsize,
	jarray pdx,
	jint pdxsize
);

JNIAPI(void, mxdrvg)(
	JNIEnv *env,
	jclass cls, 
	jint arg1,
	jint arg2
);

JNIAPI(jint, mxdrvgMeasurePlayTime)(
	JNIEnv *env,
	jclass cls, 
	jint loop,
	jint fadeout
);

JNIAPI(void, mxdrvgPlayAt)(
	JNIEnv *env,
	jclass cls, 
	jint playat,
	jint loop,
	jint fadeout
);

JNIAPI(jint, mxdrvgGetPlayAt)(
	JNIEnv *env,
	jclass cls, 
	jint dummy
);

JNIAPI(jint, mxdrvgGetTerminated)(
	JNIEnv *env,
	jclass cls, 
	jint dummy
);

JNIAPI(void, mxdrvgTotalVolume)(
	JNIEnv *env,
	jclass cls, 
	jint vol
);

JNIAPI(jint, mxdrvgGetTotalVolume)(
	JNIEnv *env,
	jclass cls, 
	jint vol
);

JNIAPI(jint, mxdrvgGetChannelMask)(
	JNIEnv *env,
	jclass cls, 
	jint dummy
);

JNIAPI(void, mxdrvgChannelMask)(
	JNIEnv *env,
	jclass cls, 
	jint mask
);

#ifdef __cplusplus
}
#endif

#endif  // __java_net_gorry_mxdrvg_jniwrap_h__

// [EOF]

