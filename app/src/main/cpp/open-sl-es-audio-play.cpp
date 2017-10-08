//
// Created by 张瑞 on 2017/10/7.
//
#include "com_example_emery_ffmpeg_AudioPlayer.h"
#include "my-log.h"
#include <jni.h>
extern "C"{
#include "libswresample/swresample.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
}

AVFormatContext* avFormatContext;
AVCodecContext* avCodecContext;
AVPacket* avPacket;
AVFrame* avFrame;
AVCodec* avCodec;
SwrContext* swrContext;
uint8_t * outbuff;
int nb_channels;
int audio_stream_idx=-1;

int createFFMpeg(const char* inputPath,int* in_nb_channels, int* in_sample_rate){
      av_register_all();
      avFormatContext=avformat_alloc_context();
      if(avformat_open_input(&avFormatContext,inputPath,NULL,NULL)<0){
          LOGE("打开失败");
          return 0;
      }

      if(avformat_find_stream_info(avFormatContext,NULL)<0){
          LOGE("获取流信息失败");
          return 0;
      }

    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if(avFormatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
            audio_stream_idx=i;
            break;
        }
    }

    avCodecContext=avFormatContext->streams[audio_stream_idx]->codec;
    avCodec=avcodec_find_decoder(avCodecContext->codec_id);

    if(avcodec_open2(avCodecContext,avCodec,NULL)<0){
        LOGE("解码信息失败");
        return 0;
    }

    avPacket= (AVPacket *) av_malloc(sizeof(AVPacket));
    avFrame=av_frame_alloc();


    swrContext=swr_alloc();
    swr_alloc_set_opts(swrContext,AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                       avCodecContext->channel_layout,avCodecContext->sample_fmt,avCodecContext->sample_rate, 0,NULL);
    swr_init(swrContext);

    outbuff= (uint8_t *) av_malloc(44100 * 2);
    nb_channels=av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

    *in_nb_channels=avCodecContext->channels;
    *in_sample_rate=avCodecContext->sample_rate;
    return 0;

}

int getPcmFrame(void** pcm,size_t* pcm_size){
    int got_frame=-1;
    while (av_read_frame(avFormatContext,avPacket)>=0){
        if(avPacket->stream_index==audio_stream_idx){
            avcodec_decode_audio4(avCodecContext,avFrame,&got_frame,avPacket);
            if(got_frame){
                swr_convert(swrContext, &outbuff,44100*2, (const uint8_t **) avFrame->data, avFrame->nb_samples);
                int size=av_samples_get_buffer_size(NULL,nb_channels,avFrame->nb_samples,AV_SAMPLE_FMT_S16,1);
                *pcm=outbuff;
                *pcm_size=size;
                break;
            }
        }

    }
    return 0;
}
int releaseFFmepg(){
    av_free_packet(avPacket);
    av_frame_free(&avFrame);
    avcodec_close(avCodecContext);
    swr_free(&swrContext);
    avformat_free_context(avFormatContext);

    return 0;
}

SLObjectItf  engineObject;
SLEngineItf  engineItf;
SLObjectItf  outputMixObject;
SLEnvironmentalReverbItf  outputMixEnvironment;
SLEnvironmentalReverbSettings outputEnvironmentSettings=SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
SLObjectItf  playerObject;
SLPlayItf  playItf;
SLAndroidSimpleBufferQueueItf androidBufferQueueItf;
void * buffer;
SLVolumeItf volumeItf;

void playerCallback(SLAndroidSimpleBufferQueueItf queueItf,void* context){
    size_t  bufferSize=0;
    getPcmFrame(&buffer,&bufferSize);
    if(bufferSize!=0&&buffer!=NULL){
        (*androidBufferQueueItf)->Enqueue(androidBufferQueueItf,buffer,bufferSize);
        LOGI("正在播放...");
    }
}

JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_AudioPlayer_openElESPlay
(JNIEnv *env, jobject instance, jstring input_){
    const char* inputStr=  env->GetStringUTFChars(input_,JNI_FALSE);
    SLresult result;

    slCreateEngine(&engineObject,0,NULL,0,NULL,NULL);
    (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineItf);

    (*engineItf)->CreateOutputMix(engineItf,&outputMixObject,0,0,0);
    (*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    result=(*outputMixObject)->GetInterface(outputMixObject,SL_IID_ENVIRONMENTALREVERB,&outputMixEnvironment);
    if(result==SL_RESULT_SUCCESS){
        (*outputMixEnvironment)->SetEnvironmentalReverbProperties(outputMixEnvironment,&outputEnvironmentSettings);
        LOGI("设置混音成功 %d",result);
    }


    int in_nb_channels;
    int in_sample_rate;
    createFFMpeg(inputStr,&in_nb_channels,&in_sample_rate);


    SLDataLocator_AndroidBufferQueue androidBufferQueue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM slDataFormat_pcm={SL_DATAFORMAT_PCM, //PCM格式
                                       in_nb_channels,//通道数
                                       SL_SAMPLINGRATE_44_1,//采样率
                                       SL_PCMSAMPLEFORMAT_FIXED_16,//采样位数
                                       SL_PCMSAMPLEFORMAT_FIXED_16,//包含位数
                                       SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,//立体声
                                       SL_BYTEORDER_LITTLEENDIAN//结束标记
                                        };
    SLDataSource slDataSource={&androidBufferQueue,&slDataFormat_pcm};


    SLDataLocator_OutputMix outputMixLocator={SL_DATALOCATOR_OUTPUTMIX,outputMixObject};
    SLDataSink slDataSink={&outputMixLocator,NULL};


    SLInterfaceID  id[3]={SL_IID_BUFFERQUEUE,SL_IID_EFFECTSEND,SL_IID_VOLUME};
    SLboolean lboolean[3]={SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};

    (*engineItf)->CreateAudioPlayer(engineItf,&playerObject,&slDataSource,&slDataSink,3,id,lboolean);
    (*playerObject)->Realize(playerObject,SL_BOOLEAN_FALSE);
    (*playerObject)->GetInterface(playerObject,SL_IID_PLAY,&playItf);
    (*playerObject)->GetInterface(playerObject,SL_IID_VOLUME,&volumeItf);
    (*playerObject)->GetInterface(playerObject,SL_IID_BUFFERQUEUE,&androidBufferQueueItf);

    (*androidBufferQueueItf)->RegisterCallback(androidBufferQueueItf,playerCallback,NULL);
    (*playItf)->SetPlayState(playItf,SL_PLAYSTATE_PLAYING);
    playerCallback(androidBufferQueueItf,NULL);

    env->ReleaseStringUTFChars(input_,inputStr);
}

JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_AudioPlayer_stopELESPlay
        (JNIEnv *env, jobject instance){
    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (playerObject != NULL) {
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
        playItf = NULL;
        androidBufferQueueItf = NULL;
        volumeItf = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironment = NULL;
    }
    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineItf = NULL;
    }
    // 释放FFmpeg解码器相关资源
    releaseFFmepg();


}
