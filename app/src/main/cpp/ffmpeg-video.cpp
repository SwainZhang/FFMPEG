//
// Created by 张瑞 on 2017/10/12.
//

#include "ffmpeg-video.h"

static void (*video_callback)(AVFrame *avFrame);

FFmpegVideo::FFmpegVideo() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

FFmpegVideo::~FFmpegVideo() {

}

int FFmpegVideo::get(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex);
    while (isPlay) {
        if (!queue.empty()) {
            //0 false success
            if (av_packet_ref(avPacket, queue.front())) {
                LOGE("获取视频帧失败");
                break;
            }

            //取出成功应该销毁这一帧数据,队列指向下一帧
            AVPacket *pkt = queue.front();
            queue.pop();
            av_free_packet(pkt);

            break;

        } else {
            pthread_cond_wait(&cond, &mutex);
        }

        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

int FFmpegVideo::put(AVPacket *avPacket) {
    AVPacket *avPacket1 = (AVPacket *) av_mallocz(sizeof(AVPacket));

    if (av_packet_ref(avPacket1, avPacket)) {
        LOGE("复制视频帧失败");
        return 0;
    }

    LOGI("压入一帧视频帧");
    pthread_mutex_lock(&mutex);
    queue.push(avPacket1);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return 1;
}

void *play_video(void *arg) {
    FFmpegVideo *video = (FFmpegVideo *) arg;

    AVPacket *avPacket = (AVPacket *) av_mallocz(sizeof(AVPacket));
    AVFrame *avFrame = av_frame_alloc();
    SwsContext *swsContext = sws_getContext(
            video->avCodecContext->width, video->avCodecContext->height,
            video->avCodecContext->pix_fmt,
            video->avCodecContext->width, video->avCodecContext->height,
            video->avCodecContext->pix_fmt,
            SWS_BILINEAR, NULL, NULL, 0);

    AVFrame *rgbFrame = av_frame_alloc();
    uint8_t *out_buffer = (uint8_t *) av_mallocz(avpicture_get_size(AV_PIX_FMT_ARGB, video->avCodecContext->width, video->avCodecContext->height));
    avpicture_fill((AVPicture *) avFrame, out_buffer, AV_PIX_FMT_ARGB, video->avCodecContext->width, video->avCodecContext->height);

    int got_frame = -1;

    double  last_play  //上一帧的播放时间
    ,play              //当前帧的播放时间
    , last_delay       // 上一次播放视频的两帧视频间隔时间
    ,delay             //两帧视频间隔时间
    ,audio_clock       //音频轨道 实际播放时间
    ,diff              //音频帧与视频帧相差时间
    ,sync_threshold    //两帧间隔合理间隔时间
    ,start_time        //从第一帧开始的绝对时间
    ,actual_delay;     //真正需要延迟时间;

   //第一帧的时间
    start_time = av_gettime() / 1000 * 1000;//us单位换成s
    while (video->isPlay) {
        video->get(avPacket);
        avcodec_decode_video2(video->avCodecContext, avFrame, &got_frame, avPacket);

        if (!got_frame) {
            continue;
        }
        sws_scale(swsContext, (const uint8_t *const *) avFrame->data, avFrame->linesize, 0,
                  avFrame->height, rgbFrame->data, rgbFrame->linesize);

        int64_t pts = av_frame_get_best_effort_timestamp(avFrame);//当前一帧的pts 时间 AVRational（n，60）
        if (pts == AV_NOPTS_VALUE) {
            pts = 0;
        }

        //如果pts=0需要修正

        //相对于第一帧的播放时间(单位为s)
        play = pts * av_q2d(video->time_base); //AVRational（n，60） pts=n/60 -->n*16ms

        //后一帧的播放时间 - 前一帧的播放时间 = 播放时延 实际上就是线程sleep的时间，
        delay = play - last_play;
        if (delay < 0 || delay > 1) {//单位为s
            delay=last_delay;
        }

        //音频的播放时间
        audio_clock=video->audio->clock;
        last_play = play;
        last_delay=delay;

        diff =video->clock-audio_clock;//diff<0 视频比音频慢
        sync_threshold=(delay>0.01?0.01:delay);//10ms

        if(fabs(diff)<10){
            if(diff<-sync_threshold){
                //视频慢了  视频需要加快
                delay=0;
            }else if(diff>sync_threshold){
                //视频快了  视频需要慢
                delay=2*delay;
            }
        }


        start_time+=delay;

        actual_delay=start_time-av_gettime()/1000*1000.000;
        if(actual_delay<0.01){
            actual_delay=0.01;
        }

        av_usleep(actual_delay*1000*1000.00+5000);
        video_callback(avFrame);

    }
}

void FFmpegVideo::play() {
    isPlay = 1;
    pthread_create(p_play_id, NULL, play_video, this);
}

void FFmpegVideo::stop() {

}

void FFmpegVideo::setAVCodecContext(AVCodecContext *avCodecContext) {
    this->avCodecContext = avCodecContext;
}

AVCodecContext *FFmpegVideo::getAVCodecContext() {
    return NULL;
}

void FFmpegVideo::setPlayCallback(void (*video_call)(AVFrame *)) {
    video_callback = video_call;
}

void FFmpegVideo::setFFmpegAudio(FFmpegAudio *audio) {
         this->audio=audio;
}

double FFmpegVideo::synchronize(AVFrame *avFrame, double play) {
    if(play!=0){
        clock=play;
    }else{
        play=clock;
    }

    double frame_rate=av_q2d(avCodecContext->time_base);//帧率 framerate
    double fps=1/frame_rate;
    double delay=avFrame->repeat_pict/(2*fps)+frame_rate;
    clock+=delay;
    return play;
}
