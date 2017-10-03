//
// Created by 张瑞 on 2017/10/3.
//
#include "com_example_emery_ffmpeg_VideoPlayer.h"
#include "my-log.h"
#include "com_example_emery_ffmpeg_MyViedeoView.h"
extern "C" {
#include "../../../libs/include/libavcodec/avcodec.h"
#include "../../../libs/include/libavformat/avformat.h"
#include "../../../libs/include/libswscale/swscale.h"
#include "../../../libs/include/libavutil/imgutils.h"
#include "android/native_window_jni.h"
#include <unistd.h>
}

JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_VideoPlayer_convertYuv
        (JNIEnv *env, jobject instance, jstring intput_, jstring output_) {

    const char *inputStr = env->GetStringUTFChars(intput_, JNI_FALSE);
    const char *outputStr = env->GetStringUTFChars(output_, JNI_FALSE);


    //    注册各大组件
    av_register_all();

    AVFormatContext *pContext = avformat_alloc_context();


    if (avformat_open_input(&pContext, inputStr, NULL, NULL) < 0) {
        LOGE("打开失败");
        return;
    }
    if (avformat_find_stream_info(pContext, NULL) < 0) {
        LOGE("获取信息失败");
        return;
    }

    int video_stream_idx = -1;
    for (int i = 0; i < pContext->nb_streams; ++i) {
        if (pContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
        }
    }

    AVCodecContext *pCodecCtx = pContext->streams[video_stream_idx]->codec;

    AVCodec *pCodex = avcodec_find_decoder(pCodecCtx->codec_id);
    if (avcodec_open2(pCodecCtx, pCodex, NULL) < 0) {
        LOGE("解码失败");
        return;
    }
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    AVFrame *frame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();
    uint8_t *out_buffer = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture *) yuvFrame, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    SwsContext *swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                            SWS_BILINEAR, NULL, NULL, NULL
    );
    FILE *fp_yuv = fopen(outputStr, "wb");
    int got_frame;
    while (av_read_frame(pContext, packet) >= 0) {

        avcodec_decode_video2(pCodecCtx, frame, &got_frame, packet);
        if (got_frame > 0) {
            sws_scale(swsContext, (const uint8_t *const *) frame->data, frame->linesize, 0,
                      frame->height, yuvFrame->data,
                      yuvFrame->linesize
            );
            int y_size = pCodecCtx->width * pCodecCtx->height;
            fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);
            fwrite(yuvFrame->data[1], 1, y_size / 4, fp_yuv);
            fwrite(yuvFrame->data[2], 1, y_size / 4, fp_yuv);
        }
        av_free_packet(packet);
    }
    fclose(fp_yuv);
    av_frame_free(&frame);
    av_frame_free(&yuvFrame);
    avcodec_close(pCodecCtx);
    avformat_free_context(pContext);


    env->ReleaseStringUTFChars(intput_, inputStr);
    env->ReleaseStringUTFChars(output_, outputStr);

}


JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_MyViedeoView_render
        (JNIEnv *env, jobject instance, jstring intput_, jobject surface){
    const  char* inputStr=env->GetStringUTFChars(intput_,JNI_FALSE);

    av_register_all();

    AVFormatContext* avFormatContext=avformat_alloc_context();

    if(avformat_open_input(&avFormatContext,inputStr,NULL,NULL)<0){
        LOGE("打开失败");
        return;
    }

    if(avformat_find_stream_info(avFormatContext,NULL)<0){
        LOGE("获取流信息失败");
        return;
    }
    int video_stream_idx=-1;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if(avFormatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
            video_stream_idx=i;
            break;
        }
    }

    AVCodecContext* avCodecContext=avFormatContext->streams[video_stream_idx]->codec;
    AVCodec* avCodec=avcodec_find_decoder(avCodecContext->codec_id);

    if(avcodec_open2(avCodecContext,avCodec,NULL)<0){
        LOGE("解码失败");
        return;
    }

    AVPacket* avPacket= (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame* avFrame=av_frame_alloc();
    int got_frame=-1;

    AVFrame* argbFrame=av_frame_alloc();
    int bufSize= avpicture_get_size(AV_PIX_FMT_RGBA,avCodecContext->width,avCodecContext->height);
    uint8_t * outBuffer= (uint8_t *) av_malloc(bufSize);
    avpicture_fill((AVPicture *) argbFrame, outBuffer, AV_PIX_FMT_RGBA, avCodecContext->width, avCodecContext->height);


    SwsContext* swsContext=sws_getContext(avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
                                          avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA,
                                          SWS_BICUBIC, NULL,NULL,NULL);

    ANativeWindow* aNativeWindow=ANativeWindow_fromSurface(env,surface);
    ANativeWindow_Buffer outBuff;
    while (av_read_frame(avFormatContext,avPacket)>=0){

        if(avPacket->stream_index==video_stream_idx) {
            avcodec_decode_video2(avCodecContext, avFrame, &got_frame, avPacket);

            if (got_frame) {

                ANativeWindow_setBuffersGeometry(aNativeWindow, avCodecContext->width,
                                                 avCodecContext->height, WINDOW_FORMAT_RGBA_8888);
                ANativeWindow_lock(aNativeWindow, &outBuff, NULL);

                sws_scale(swsContext, (const uint8_t *const *) avFrame->data, avFrame->linesize, 0,
                          avFrame->height, argbFrame->data, argbFrame->linesize);

                //outBuff的首地址
                uint8_t *dst = (uint8_t *) outBuff.bits;
                int dstStride = outBuff.stride * 4;

                //argbFrame的首地址
                uint8_t *src = argbFrame->data[0];
                int srcStride = argbFrame->linesize[0];

                for (int i = 0; i < avCodecContext->height; ++i) {
                    memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
                }

                ANativeWindow_unlockAndPost(aNativeWindow);
                usleep(1000 * 16);
            }
        }
        av_free_packet(avPacket);

    }

    ANativeWindow_release(aNativeWindow);
    av_frame_free(&avFrame);
    av_frame_free(&argbFrame);
    avcodec_close(avCodecContext);
    avformat_free_context(avFormatContext);



    env->ReleaseStringUTFChars(intput_,inputStr);
}