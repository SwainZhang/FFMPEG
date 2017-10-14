//
// Created by 张瑞 on 2017/10/12.
//

#include "ffmpeg-video.h"

FFmpegVideo::FFmpegVideo() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

FFmpegVideo::~FFmpegVideo() {

}

int FFmpegVideo::get(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex);
    while (isPlay){
        if(!queue.empty()){
            //0 false success
            if(av_packet_ref(avPacket,queue.front())){
                LOGE("获取视频帧失败");
                break;
            }

            //取出成功应该销毁这一帧数据,队列指向下一帧
            AVPacket* pkt=queue.front();
            queue.pop();
            av_free_packet(pkt);

            break;

        }else{
            pthread_cond_wait(&cond,&mutex);
        }

        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

int FFmpegVideo::put(AVPacket *avPacket) {
    AVPacket* avPacket1= (AVPacket *) av_malloc(sizeof(AVPacket));

    if(av_packet_ref(avPacket1,avPacket)){
        LOGE("复制视频帧失败");
    }

    LOGI("压入一帧视频帧");
    pthread_mutex_lock(&mutex);
    queue.push(avPacket1);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return 1;
}
void * play_video(void* arg){
    FFmpegVideo* video= (FFmpegVideo *) arg;

    AVPacket* avPacket= (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame* avFrame=av_frame_alloc();
    SwsContext* swsContext=sws_getContext(
            video->avCodecContext->width, video->avCodecContext->height, video->avCodecContext->pix_fmt,
            video->avCodecContext->width, video->avCodecContext->height,video->avCodecContext->pix_fmt,
            SWS_BILINEAR,NULL,NULL,0);

    AVFrame* dst=av_frame_alloc();
    uint8_t* out_buffer= (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_ARGB, video->avCodecContext->width, video->avCodecContext->height));
    avpicture_fill((AVPicture *) avFrame, out_buffer, AV_PIX_FMT_ARGB, video->avCodecContext->width, video->avCodecContext->height);

    int got_frame=-1;
    while (video->isPlay){
        video->get(avPacket);
        avcodec_decode_video2(video->avCodecContext,avFrame,&got_frame,avPacket);

        if(got_frame){
            sws_scale(swsContext, (const uint8_t *const *) avFrame->data, avFrame->linesize, 0, avFrame->height, dst->data, dst->linesize);
        }

    }
}
void FFmpegVideo::play() {
    isPlay=1;
    pthread_create(p_play_id,NULL,play_video,this);
}

void FFmpegVideo::stop() {

}

void FFmpegVideo::setAVCodecContext(AVCodecContext *avCodecContext) {
   this->avCodecContext=avCodecContext;
}

AVCodecContext *FFmpegVideo::getAVCodecContext() {
    return NULL;
}
