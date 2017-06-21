#include <jni.h>
#include "encodeVideoOnly.h"

struct CallbackStruct {
    JNIEnv *env;
    jobject obj;
    jmethodID mProgress;
    jmethodID mDone;
    jmethodID mError;
};

void progressImpl(void *vp, int val) {
    struct CallbackStruct *cs = (struct CallbackStruct *) vp;
    (*(cs->env))->CallVoidMethod(cs->env, cs->obj, cs->mProgress, val);
}

void doneImpl(void *vp) {
    struct CallbackStruct *cs = (struct CallbackStruct *) vp;
    (*(cs->env))->CallVoidMethod(cs->env, cs->obj, cs->mDone);
}

void errorImpl(void *vp, char *err) {
    struct CallbackStruct *cs = (struct CallbackStruct *) vp;
    //jstring str = (*(cs->env))->NewStringUTF(cs->env, err);
    (*(cs->env))->CallVoidMethod(cs->env, cs->obj, cs->mError);
}

JNIEXPORT void JNICALL
Java_com_rnffmpeg_FFMpeg_encodeVideoOnly(JNIEnv *env, jobject obj, jstring filename_,
                                     jstring outputFile_) {
    const char *filename = (*env)->GetStringUTFChars(env, filename_, 0);
    const char *outputFile = (*env)->GetStringUTFChars(env, outputFile_, 0);
    jclass cls = (*env)->GetObjectClass(env, obj);
    struct CallbackStruct cs = {
            env,
            obj,
            (*env)->GetMethodID(env, cls, "progress", "(I)V"),
            (*env)->GetMethodID(env, cls, "done", "()V"),
            (*env)->GetMethodID(env, cls, "error", "()V")
    };

    encodeVideoOnly(filename, outputFile, &cs, &progressImpl, &doneImpl, &errorImpl);

    (*env)->ReleaseStringUTFChars(env, filename_, filename);
    (*env)->ReleaseStringUTFChars(env, outputFile_, outputFile);
}