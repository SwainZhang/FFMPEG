//
// Created by 张瑞 on 2017/10/8.
//
#include "com_example_emery_ffmpeg_EmeryPlayer.h"
#include "my-log.h"
#include "ffmepg-audio.h"
#include "ffmpeg-video.h"

FFmpegVideo *fFmpegVideo = NULL;
FFmpegAudio *fFmpegAudio = NULL;
pthread_t p_tid;
const char *inputStr = NULL;
int isPlay = -1;
/**
 * 解码线程：生产者
 * @param args
 * @return
 */
void *process(void *args) {
    av_register_all();

    AVFormatContext *avFormatContext = avformat_alloc_context();

    if (avformat_open_input(&avFormatContext, inputStr, NULL, NULL) < 0) {
        LOGE("打开失败");
        return;
    }

    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        LOGE("获取流信息失败");
        return;
    }

    for (int i = 0; i < avFormatContext->nb_streams; ++i) {

        AVCodecContext *avCodecContext = avFormatContext->streams[i]->codec;
        AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);

        if (avcodec_open2(avCodecContext, avCodec, NULL) < 0) {
            LOGE("无法打开解码器");
        }

        if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            fFmpegVideo->setAVCodecContext(avCodecContext);
            fFmpegVideo->index = i;

        } else if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            fFmpegAudio->setAVCodecContext(avCodecContext);
            fFmpegAudio->index = i;

        }
    }

    fFmpegVideo->play();
    fFmpegAudio->play();

    isPlay = 1;

    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));

    //解码线程，生产者，生产视频packet和音频packet
    while (isPlay && av_read_frame(avFormatContext, avPacket) == 0) {

        if (fFmpegVideo && fFmpegVideo->isPlay && avPacket->stream_index == fFmpegVideo->index) {
            fFmpegVideo->put(avPacket);
        } else if (fFmpegAudio && fFmpegAudio->isPlay && avPacket->stream_index == fFmpegVideo->index) {
            fFmpegVideo->put(avPacket);
        }
        av_packet_unref(avPacket);
    }

    isPlay=0;

    if(fFmpegVideo&&fFmpegVideo->isPlay){
        fFmpegVideo->stop();
    }

    if(fFmpegAudio&&fFmpegAudio->isPlay){
        fFmpegAudio->stop();
    }

    av_free_packet(avPacket);
    avformat_free_context(avFormatContext);
}

JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_EmeryPlayer_audioVideoPlay
        (JNIEnv *env, jobject instance, jstring input_) {
    inputStr = env->GetStringUTFChars(input_, JNI_FALSE);

    fFmpegVideo = new FFmpegVideo();
    fFmpegAudio = new FFmpegAudio();
    pthread_create(&p_tid, NULL, process, NULL);


    env->ReleaseStringUTFChars(input_, inputStr);
}

/*
 * Class:     com_example_emery_ffmpeg_EmeryPlayer
 * Method:    player
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_EmeryPlayer_player
        (JNIEnv *env, jobject instance) {

}


/*
 * Class:     com_example_emery_ffmpeg_EmeryPlayer
 * Method:    stop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_EmeryPlayer_stop
        (JNIEnv *env, jobject instance) {

}

/*
 * Class:     com_example_emery_ffmpeg_EmeryPlayer
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_EmeryPlayer_release
        (JNIEnv *env, jobject instance) {

}

/*
 * Class:     com_example_emery_ffmpeg_EmeryPlayer
 * Method:    display
 * Signature: (Landroid/view/Surface;)V
 */
JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_EmeryPlayer_display
        (JNIEnv *env, jobject instance, jobject surface) {

}

