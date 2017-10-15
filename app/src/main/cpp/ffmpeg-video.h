//
// Created by 张瑞 on 2017/10/12.
//

#ifndef FFMPEG_FFMPEG_VIDEO_H
#define FFMPEG_FFMPEG_VIDEO_H

#include "header.h"
#include "ffmepg-audio.h"

class FFmpegVideo{
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

    //pts
    AVRational time_base;

    //音频 视频追音频
    FFmpegAudio* audio;

    //视频的播放时间线
    double clock;
public:
    FFmpegVideo();
    ~FFmpegVideo();
    int get(AVPacket *avPacket);
    int put(AVPacket *avPacket);
    void play();
    void stop();
    void setAVCodecContext( AVCodecContext* avCodecContext);
    void setFFmpegAudio(FFmpegAudio* audio);
    void setPlayCallback(void(*video_callback)(AVFrame* avFrame));
    double synchronize(AVFrame* avFrame,double play);
    AVCodecContext* getAVCodecContext();

};
#endif //FFMPEG_FFMPEG_VIDEO_H
