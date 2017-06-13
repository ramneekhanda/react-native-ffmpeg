#include <jni.h>
#include "make_gif.h"

JNIEXPORT jint JNICALL
Java_com_rnffmpeg_FFMpeg_encodeVideo(JNIEnv *env, jclass type, jstring filename_,
                                     jstring outputfile_) {
    const char *filename = (*env)->GetStringUTFChars(env, filename_, 0);
    const char *outputfile = (*env)->GetStringUTFChars(env, outputfile_, 0);

    int ret = make_gif(filename, outputfile);

    (*env)->ReleaseStringUTFChars(env, filename_, filename);
    (*env)->ReleaseStringUTFChars(env, outputfile_, outputfile);
    return ret;
}