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


ANativeWindow* nativeWindow;

void video_play_callback(AVFrame* frame){

    if(nativeWindow==NULL){
        return;
    }

    ANativeWindow_Buffer window_buffer;

    if(ANativeWindow_lock(nativeWindow,&window_buffer,0)){
        return;
    }
    LOGE("绘制 宽%d,高%d",frame->width,frame->height);
    LOGE("绘制 宽%d,高%d  行字节 %d ",window_buffer.width,window_buffer.height, frame->linesize[0]);

    //RGBA帧的起始地址
    uint8_t *dst= (uint8_t *) window_buffer.bits;
    int dstStride=window_buffer.stride;

    //视频帧的起始地址
    uint8_t *src=frame->data[0];
    int srcStride=frame->linesize[0];

    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst+i*dstStride,src+i*srcStride,srcStride);
    }
    ANativeWindow_unlockAndPost(nativeWindow);

}

/**
 * 解码线程：生产者
 * @param args
 * @return
 */
void *process(void *args) {

    av_register_all();

    //播放网络
    avformat_network_init();

    AVFormatContext *avFormatContext = avformat_alloc_context();

    if (avformat_open_input(&avFormatContext, inputStr, NULL, NULL) < 0) {
        LOGE("打开失败");
    }

    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        LOGE("获取流信息失败");
    }

    for (int i = 0; i < avFormatContext->nb_streams; ++i) {

        AVCodecContext *avCodecContext = avFormatContext->streams[i]->codec;
        AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);

        AVCodecContext* avCodecCtx=avcodec_alloc_context3(avCodec);
        avcodec_copy_context(avCodecCtx,avCodecContext);

        if (avcodec_open2(avCodecCtx, avCodec, NULL) < 0) {
            LOGE("无法打开解码器");
            continue;
        }

        if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            fFmpegVideo->setAVCodecContext(avCodecCtx);
            fFmpegVideo->index = i;

        } else if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            fFmpegAudio->setAVCodecContext(avCodecCtx);
            fFmpegAudio->index = i;
            fFmpegAudio->time_base=avCodecCtx->time_base;
        }
    }

    //视频追随音频同步
    fFmpegVideo->setFFmpegAudio(fFmpegAudio);

    //开启播放线程
    fFmpegVideo->play();
    fFmpegAudio->play();

    isPlay = 1;

    AVPacket *avPacket = (AVPacket *) av_mallocz(sizeof(AVPacket));

    //解码线程，生产者，生产视频packet和音频packet
    while (isPlay) {

        SLresult ret=av_read_frame(avFormatContext, avPacket);

        if(ret==0){
            if (fFmpegVideo && fFmpegVideo->isPlay && avPacket->stream_index == fFmpegVideo->index) {
                fFmpegVideo->put(avPacket);
            } else if (fFmpegAudio && fFmpegAudio->isPlay && avPacket->stream_index == fFmpegVideo->index) {
                fFmpegVideo->put(avPacket);
            }
            av_packet_unref(avPacket);

        } else if(ret==AVERROR_EOF){
            //读取完毕，不一定播放完成
            while (isPlay){
                if (fFmpegAudio&&fFmpegAudio->queue.empty() && fFmpegVideo&&fFmpegVideo->queue.empty()) {
                    break;
                }
                av_usleep(10000);
            }
        }

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

    pthread_exit(0);
}

JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_EmeryPlayer_audioVideoPlay
        (JNIEnv *env, jobject instance, jstring input_) {
    inputStr = env->GetStringUTFChars(input_, JNI_FALSE);

    fFmpegVideo = new FFmpegVideo();
    fFmpegAudio = new FFmpegAudio();

    fFmpegVideo->setPlayCallback(video_play_callback);
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
      if(nativeWindow!=NULL){
          ANativeWindow_release(nativeWindow);
          nativeWindow=NULL;
      }
      nativeWindow=ANativeWindow_fromSurface(env,surface);
      if(fFmpegVideo!=NULL&&fFmpegVideo->avCodecContext!=NULL){
          ANativeWindow_setBuffersGeometry(nativeWindow,fFmpegVideo->avCodecContext->width,fFmpegVideo->avCodecContext->height,WINDOW_FORMAT_RGBA_8888);
      }
}

