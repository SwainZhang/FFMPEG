//
// Created by 张瑞 on 2017/10/8.
//
#include "com_example_emery_ffmpeg_EmeryPlayer.h"
#include "my-log.h"

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}
JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_EmeryPlayer_audioVideoPlay
(JNIEnv *env, jobject instance, jstring input_){
    const char *inputStr=env->GetStringUTFChars(input_,JNI_FALSE);



    env->ReleaseStringUTFChars(input_,inputStr);
}