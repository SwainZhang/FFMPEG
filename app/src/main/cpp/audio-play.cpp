//
// Created by 张瑞 on 2017/10/3.
//
#include <jni.h>
#include "com_example_emery_ffmpeg_AudioPlayer.h"
#include "my-log.h"
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}
JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_AudioPlayer_convertPCM
(JNIEnv *env, jobject instance, jstring intput_, jstring output_){
    const char *inputStr=env->GetStringUTFChars(intput_,JNI_FALSE);
    const char* outputStr= env->GetStringUTFChars(output_,JNI_FALSE);
     LOGI("---------1---------");

    av_register_all();

    AVFormatContext *avFormatContext=avformat_alloc_context();

    if(avformat_open_input(&avFormatContext,inputStr,NULL,NULL)<0){
        LOGE("打开失败");
        return;
    }
    LOGI("---------2---------");
    if(avformat_find_stream_info(avFormatContext,NULL)<0){
        LOGE("获取流信息失败");
        return;
    }
    LOGI("---------3---------");
    int audio_stream_idx=-1;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if(avFormatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
            audio_stream_idx=i;
            break;
        }
    }
    LOGI("---------4---------");
    AVCodecContext* avCodecContext=avFormatContext->streams[audio_stream_idx]->codec;
    AVCodec* avCodec=avcodec_find_decoder(avCodecContext->codec_id);

    if(avcodec_open2(avCodecContext,avCodec,NULL)<0){
        LOGE("初始化解码失败");
        return;
    }

    LOGI("---------5---------");
    AVPacket* avPacket= (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(avPacket);
    AVFrame* avFrame=av_frame_alloc();
    int got_frame;
    LOGI("---------6---------");
    SwrContext* swrContext=swr_alloc();
    swr_alloc_set_opts(swrContext,
                       AV_CH_LAYOUT_STEREO,//立体声
                       AV_SAMPLE_FMT_S16,//16位
                       44100,
                       avCodecContext->channel_layout,
                       avCodecContext->sample_fmt,
                       avCodecContext->sample_rate,0,NULL);
    swr_init(swrContext);
    LOGI("---------7---------");
    //44100 * 2  sample_rate 双通道（16bit = 2 byte)
    uint8_t * outBuff= (uint8_t *) av_malloc(44100 * 2);

    int nb_channels=av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

    FILE* file_pcm=fopen(outputStr,"wb");
    LOGI("---------8---------");
    while (av_read_frame(avFormatContext,avPacket)>=0){
        if(avPacket->stream_index==audio_stream_idx){
            avcodec_decode_audio4(avCodecContext,avFrame,&got_frame,avPacket);
            if(got_frame){
                LOGI("---------9---------");
                swr_convert(swrContext, &outBuff,44100*2, (const uint8_t **) avFrame->data, avFrame->nb_samples);

                //outBuffer参数要和SwrContext输出参数匹配
                int size=av_samples_get_buffer_size(NULL,nb_channels,avFrame->nb_samples,AV_SAMPLE_FMT_S16,1);

                fwrite(outBuff,1,size,file_pcm);
            }

        }
        av_free_packet(avPacket);
    }
    fclose(file_pcm);
    av_frame_free(&avFrame);
    avcodec_close(avCodecContext);
    swr_free(&swrContext);
    avformat_free_context(avFormatContext);

    env->ReleaseStringUTFChars(intput_,inputStr);
    env->ReleaseStringUTFChars(output_,outputStr);

}

JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_AudioPlayer_directPlay
        (JNIEnv *env, jobject instance, jstring intput_){
   const char * inputStr=env->GetStringUTFChars(intput_,JNI_FALSE);
    LOGI("直接播放");

    av_register_all();

    AVFormatContext * avFormatContext=avformat_alloc_context();

    if(avformat_open_input(&avFormatContext,inputStr,NULL,NULL)<0){
        LOGI("打开失败");
        return;
    }
    LOGI("----------1----------");
    if(avformat_find_stream_info(avFormatContext,NULL)<0){
        LOGI("获取流信息失败");
        return;
    }
    LOGI("----------2----------");
    int audio_stream_idx=-1;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if(avFormatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
            audio_stream_idx=i;
            break;
        }
    }
    LOGI("----------3----------");
    AVCodecContext * avCodecContext=avFormatContext->streams[audio_stream_idx]->codec;
    AVCodec* avCodec=avcodec_find_decoder(avCodecContext->codec_id);

    if(avcodec_open2(avCodecContext,avCodec,NULL)<0){
        return;
    }
    LOGI("----------4----------");
    AVPacket * avPacket= (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(avPacket);
    AVFrame* avFrame=av_frame_alloc();
    int got_frame;

    SwrContext *swrContext=swr_alloc();
    swr_alloc_set_opts(swrContext,
                       AV_CH_LAYOUT_STEREO,
                       AV_SAMPLE_FMT_S16,
                       44100,
                       avCodecContext->channel_layout,
                       avCodecContext->sample_fmt,
                       avCodecContext->sample_rate,0,NULL);
    swr_init(swrContext);
    LOGI("----------5----------");
    uint8_t * outBuff= (uint8_t *) av_malloc(44100 * 2);
    int nb_channels=av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

    jclass audio_player=env->GetObjectClass(instance);
    jmethodID createAudioTrackId= env->GetMethodID(audio_player,"createAudioTrack","(I)V");
    env->CallVoidMethod(instance,createAudioTrackId,nb_channels);

    LOGI("----------6----------");
    jmethodID writeByteId=env->GetMethodID(audio_player,"writeByte","([BI)V");

    while (av_read_frame(avFormatContext,avPacket)>=0){
        if(avPacket->stream_index==audio_stream_idx) {
            avcodec_decode_audio4(avCodecContext, avFrame, &got_frame, avPacket);
            if (got_frame) {
            swr_convert(swrContext, &outBuff, 44100*2, (const uint8_t **) avFrame->data, avFrame->nb_samples);
                LOGI("----------7----------");
                int size = av_samples_get_buffer_size(NULL, nb_channels, avFrame->nb_samples, AV_SAMPLE_FMT_S16, 1);

                jbyteArray jbuffer=env->NewByteArray(size);
                env->SetByteArrayRegion(jbuffer, 0, size, (const jbyte *) outBuff);
                env->CallVoidMethod(instance,writeByteId,jbuffer,size);
                env->DeleteLocalRef(jbuffer);
        }
        }
        av_free_packet(avPacket);
    }
    LOGI("----------8----------");
    av_frame_free(&avFrame);
    avcodec_close(avCodecContext);
    swr_free(&swrContext);
    avformat_free_context(avFormatContext);

    env->ReleaseStringUTFChars(intput_,inputStr);
}