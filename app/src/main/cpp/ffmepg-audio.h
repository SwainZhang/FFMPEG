//
// Created by 张瑞 on 2017/10/12.
//

#ifndef FFMPEG_FFMEPG_AUDIO_H
#define FFMPEG_FFMEPG_AUDIO_H

#include "header.h"


class FFmpegAudio {

public :

    //是否正在播放
    int isPlay;

    //流索引
    int index;

    //音频队列
    std::queue<AVPacket*> queue;

    //处理线程
    pthread_t p_play_id;

    //解码器上下文
    AVCodecContext* avCodecContext;

    //同步锁
    pthread_mutex_t mutex;

    //条件变量
    pthread_cond_t cond;

    //转换上下文
    SwrContext* swrContext;

    //输出缓冲区
    uint8_t * out_buffer;

    //通道数
    int nb_channels;

    //从第一帧开始已经播放的时间
    double clock;

    //pts 不同场景下有不同的方式
    // AVCodecContext中的AVRational根据帧率来设定，如25帧，那么num = 1，den=25
    // AVStream中的time_base一般根据其采样频率设定，如（1，90000）
    AVRational time_base;

    SLObjectItf  engineObject;
    SLEngineItf  engineItf;
    SLObjectItf  outputMixObject;
    SLEnvironmentalReverbItf  outputMixEnvironment;
    SLObjectItf  playerObject;
    SLPlayItf  playItf;
    SLAndroidSimpleBufferQueueItf androidBufferQueueItf;
    void * buffer;
    SLVolumeItf volumeItf;

public:
    FFmpegAudio();
    ~ FFmpegAudio();
    int get(AVPacket *avPacket);
    int put(AVPacket *avPacket);
    void play();
    void stop();
    void setAVCodecContext( AVCodecContext* avCodecContext);
    AVCodecContext* getAVCodecContext();
    int  createPlayer();
};



#endif //FFMPEG_FFMEPG_AUDIO_H
