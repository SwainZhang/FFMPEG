//
// Created by 张瑞 on 2017/10/12.
//

#include "ffmepg-audio.h"


int getPcmFrame(FFmpegAudio *audio);

void playerCallback(SLAndroidSimpleBufferQueueItf androidBufferQueueItf, void *context);

int creatFFmpeg(FFmpegAudio *audio);

int FFmpegAudio::get(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex);

    while (isPlay) {
        if (!queue.empty()) {
            //从队列中取出一帧数据
            if (av_packet_ref(avPacket, queue.front())) {
                LOGE("取出音频帧失败");
                break;
            }

            //取出成功应该销毁这一帧数据,队列指向下一帧
            AVPacket *pkt = queue.front();
            queue.pop();
            av_free_packet(pkt);

            break;

        } else {
            //如果队列没有数据就阻塞
            pthread_cond_wait(&cond, &mutex);
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

int FFmpegAudio::put(AVPacket *avPacket) {
    AVPacket *avPacket1 = (AVPacket *) av_mallocz(sizeof(AVPacket));

    if (av_packet_ref(avPacket1, avPacket) != 0) {
        LOGE("复制音频帧失败");
        return 0;
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
    audio->createPlayer();

    //播放音频完成就退出线程
    pthread_exit(0);
}

void FFmpegAudio::play() {
    isPlay = 1;
    pthread_create(&p_play_id, NULL, play_audio, this);
}

void FFmpegAudio::stop() {
    LOGE("声音暂停");
    //因为可能卡在 deQueue
    pthread_mutex_lock(&mutex);
    isPlay = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_join(p_play_id, 0);

    if (playItf != NULL) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_STOPPED);
        playItf = NULL;
    }
    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (playerObject != NULL) {
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
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

    if (swrContext) {
        swr_free(&swrContext);
    }

    if (avCodecContext) {
        if (avcodec_is_open(avCodecContext)) {
            avcodec_close(avCodecContext);
        }
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    LOGE("AUDIO clear");


}

FFmpegAudio::FFmpegAudio() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    clock = 0;
}

FFmpegAudio::~FFmpegAudio() {
    if (out_buffer) {
        free(out_buffer);
    }
    for (int i = 0; i < queue.size(); ++i) {
        AVPacket *pkt = queue.front();
        queue.pop();
        LOGI("销毁音频帧%d",queue.size());
        av_free(pkt);
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

void FFmpegAudio::setAVCodecContext(AVCodecContext *avCodecContext) {
    this->avCodecContext = avCodecContext;
    creatFFmpeg(this);
}

AVCodecContext *FFmpegAudio::getAVCodecContext() {
    return this->avCodecContext;
}

int FFmpegAudio::createPlayer() {
    SLresult result;

    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);

    (*engineItf)->CreateOutputMix(engineItf, &outputMixObject, 0, 0, 0);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironment);
    if (result == SL_RESULT_SUCCESS) {
        (*outputMixEnvironment)->SetEnvironmentalReverbProperties(outputMixEnvironment,
                                                                  &outputEnvironmentSettings);
        LOGI("设置混音成功 %d", result);
    }


    SLDataLocator_AndroidBufferQueue androidBufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                           2};
    SLDataFormat_PCM slDataFormat_pcm = {SL_DATAFORMAT_PCM, //PCM格式
                                         nb_channels,//通道数
                                         SL_SAMPLINGRATE_44_1,//采样率
                                         SL_PCMSAMPLEFORMAT_FIXED_16,//采样位数
                                         SL_PCMSAMPLEFORMAT_FIXED_16,//包含位数
                                         SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声
                                         SL_BYTEORDER_LITTLEENDIAN//结束标记
    };
    SLDataSource slDataSource = {&androidBufferQueue, &slDataFormat_pcm};


    SLDataLocator_OutputMix outputMixLocator = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink slDataSink = {&outputMixLocator, NULL};


    SLInterfaceID id[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    SLboolean lboolean[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    (*engineItf)->CreateAudioPlayer(engineItf, &playerObject, &slDataSource, &slDataSink, 3, id,
                                    lboolean);
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playItf);
    (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &volumeItf);
    (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &androidBufferQueueItf);

    (*androidBufferQueueItf)->RegisterCallback(androidBufferQueueItf, playerCallback, NULL);
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    playerCallback(androidBufferQueueItf, this);

    return 0;
}

int getPcmFrame(FFmpegAudio *audio) {
    int got_frame = -1;
    AVFrame *avFrame = av_frame_alloc();
    AVPacket *avPacket = (AVPacket *) av_mallocz(sizeof(AVPacket));
    int nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    int out_buffer_size = 0;

    while (audio->isPlay) {
        audio->get(avPacket);
        if (avPacket->pts != AV_NOPTS_VALUE) {
            audio->clock = avPacket->pts * av_q2d(audio->time_base);
        }
        avcodec_decode_audio4(audio->avCodecContext, avFrame, &got_frame, avPacket);
        if (got_frame) {
            LOGI("getPcmFrame");
            swr_convert(audio->swrContext, &audio->out_buffer, 44100 * 2,
                        (const uint8_t **) avFrame->data, avFrame->nb_samples);
            out_buffer_size = av_samples_get_buffer_size(NULL, nb_channels, avFrame->nb_samples,
                                                         AV_SAMPLE_FMT_S16, 1);

            break;
        }
    }
    av_free_packet(avPacket);
    av_frame_free(&avFrame);

    return out_buffer_size;
}

void playerCallback(SLAndroidSimpleBufferQueueItf androidBufferQueueItf, void *context) {
    FFmpegAudio *audio = (FFmpegAudio *) context;
    size_t bufferSize = getPcmFrame(audio);
    if (bufferSize != 0 && audio->out_buffer != NULL) {

        //44100*2*2 采样率*通道数*采样位数=采样字节数/s
        double time = bufferSize / ((double) 44100 * 2 * 2);//每一帧的时间
        audio->clock += time;

        (*androidBufferQueueItf)->Enqueue(androidBufferQueueItf, audio->out_buffer, bufferSize);
        LOGI("正在播放音频...");
    }
}

int creatFFmpeg(FFmpegAudio *audio) {

    audio->swrContext = swr_alloc();
    swr_alloc_set_opts(audio->swrContext, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                       audio->avCodecContext->channel_layout, audio->avCodecContext->sample_fmt,
                       audio->avCodecContext->sample_rate, 0, NULL);
    swr_init(audio->swrContext);

    audio->out_buffer = (uint8_t *) av_mallocz(44100 * 2);
    audio->nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    LOGI("creatFFmpeg");

    return 0;

}



