//
// Created by 张瑞 on 2017/10/12.
//

#include "ffmepg-audio.h"

int FFmpegAudio::get(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex);

    while (isPlay){
        if(!queue.empty()){
            //从队列中取出一帧数据
              if(av_packet_ref(avPacket,queue.front())){
                  LOGE("取出音频帧失败");
                  break;
              }

            //取出成功应该销毁这一帧数据,队列指向下一帧
            AVPacket* pkt=queue.front();
            queue.pop();
            av_free_packet(pkt);

            break;

        }else{
            //如果队列没有数据就阻塞
            pthread_cond_wait(&cond,&mutex);
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

int FFmpegAudio::put(AVPacket *avPacket) {
    AVPacket *avPacket1 = (AVPacket *) av_malloc(sizeof(AVPacket));

    if (av_packet_ref(avPacket1, avPacket) != 0) {
        LOGE("复制音频帧失败");
    }

    LOGI("压入一帧音频帧");
    pthread_mutex_lock(&mutex);
    queue.push(avPacket1);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return 1;
}

void *play_audio(void *arg) {
    LOGI("开启音频线程");
    FFmpegAudio *audio = (FFmpegAudio *) arg;
    AVFrame* avFrame=av_frame_alloc();
    AVPacket* avPacket= (AVPacket *) av_malloc(sizeof(AVPacket));
    SwrContext* swrContext=swr_alloc();
    swr_alloc_set_opts(
            swrContext,AV_CH_LAYOUT_STEREO,
            AV_SAMPLE_FMT_S16,
            audio->avCodecContext->sample_rate,
            audio->avCodecContext->channel_layout,
            audio->avCodecContext->sample_fmt,
            audio->avCodecContext->sample_rate,0,0);
    swr_init(swrContext);

    uint8_t *out_buffer= (uint8_t *) av_malloc(44100 * 2);
    int nb_channels=av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    int got_frame=-1;

    while (audio->isPlay){
        audio->get(avPacket);
        avcodec_decode_audio4(audio->avCodecContext,avFrame,&got_frame,avPacket);
        if(got_frame){
            swr_convert(swrContext,&out_buffer,44100*2, (const uint8_t **) avFrame->data, avFrame->nb_samples);
            int out_buffer_size=av_samples_get_buffer_size(NULL,nb_channels,avFrame->nb_samples,AV_SAMPLE_FMT_S16,1);

        }
    }
}

void FFmpegAudio::play() {
    isPlay=1;
    pthread_create(&p_play_id, NULL, play_audio, this);
}

void FFmpegAudio::stop() {

}

FFmpegAudio::FFmpegAudio() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

FFmpegAudio::~FFmpegAudio() {

}

void FFmpegAudio::setAVCodecContext(AVCodecContext *avCodecContext) {
    this->avCodecContext = avCodecContext;
}

AVCodecContext *FFmpegAudio::getAVCodecContext() {
    return this->avCodecContext;
}
