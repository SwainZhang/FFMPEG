//
// Created by 张瑞 on 2017/10/12.
//

#ifndef FFMPEG_FFMEPG_AUDIO_H
#define FFMPEG_FFMEPG_AUDIO_H

#include "header.h"
extern "C" {
#include <queue>
};

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

public:
    FFmpegAudio();
    ~ FFmpegAudio();
    int get(AVPacket *avPacket);
    int put(AVPacket *avPacket);
    void play();
    void stop();
    void setAVCodecContext( AVCodecContext* avCodecContext);
    AVCodecContext* getAVCodecContext();
};

#endif //FFMPEG_FFMEPG_AUDIO_H
