//
// Created by 张瑞 on 2017/10/3.
//
#include "com_example_emery_ffmpeg_VideoPlayer.h"
#include "my-log.h"

extern "C" {
#include "../../../libs/include/libavcodec/avcodec.h"
#include "../../../libs/include/libavformat/avformat.h"
#include "../../../libs/include/libswscale/swscale.h"
#include "../../../libs/include/libavutil/imgutils.h"
}

JNIEXPORT void JNICALL Java_com_example_emery_ffmpeg_VideoPlayer_convertYuv
        (JNIEnv *env, jobject instance, jstring intput_, jstring output_) {

    const char *inputStr = env->GetStringUTFChars(intput_, JNI_FALSE);
    const char *outputStr = env->GetStringUTFChars(output_, JNI_FALSE);

    av_register_all();

    AVFormatContext* avFormatContext=avformat_alloc_context();

    if(avformat_open_input(&avFormatContext,inputStr,NULL,NULL)<0){
        LOGI("打开失败");
        return;
    }

    if(avformat_find_stream_info(avFormatContext,NULL)<0){
        LOGI("获取流信息失败");
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
        LOGI("解码失败");
    }

    AVPacket* avPacket= (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(avPacket);


    AVFrame* avFrame=av_frame_alloc();
    int got_frame=-1;

    AVFrame* yuvFrame=av_frame_alloc();
    uint8_t * outBuffer= (uint8_t *) av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, avCodecContext->width, avCodecContext->height, 0));
    av_image_fill_arrays(yuvFrame->data,yuvFrame->linesize,outBuffer,AV_PIX_FMT_YUV420P, avCodecContext->width, avCodecContext->height,0);

    SwsContext* swsContext=sws_getContext(avCodecContext->width,
                                          avCodecContext->height,
                                          avCodecContext->pix_fmt,
                                          avCodecContext->width,
                                          avCodecContext->height,
                                          AV_PIX_FMT_YUV420P,
                                          SWS_BILINEAR,NULL,NULL,NULL);

    FILE * file_yuv=fopen(outputStr,"wb");
    while (av_read_frame(avFormatContext,avPacket)>=0){
        avcodec_decode_video2(avCodecContext,avFrame,&got_frame,avPacket);
        if(got_frame){
            sws_scale(swsContext, (const uint8_t *const *) avFrame->data, avFrame->linesize, 0, avFrame->height, yuvFrame->data, yuvFrame->linesize);

            int yuv_size=avCodecContext->width*avCodecContext->height;
            fwrite(avFrame->data[0],1,yuv_size,file_yuv);
            fwrite(avFrame->data[1],1,yuv_size/4,file_yuv);
            fwrite(avFrame->data[2],1,yuv_size/4,file_yuv);
        }
        av_packet_free(&avPacket);

    }

    fclose(file_yuv);
    av_frame_free(&avFrame);
    av_frame_free(&yuvFrame);
    avformat_free_context(avFormatContext);


    env->ReleaseStringUTFChars(intput_,inputStr);
    env->ReleaseStringUTFChars(output_,outputStr);

}