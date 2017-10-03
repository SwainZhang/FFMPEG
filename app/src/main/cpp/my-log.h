//
// Created by 张瑞 on 2017/10/3.
//
#include <android/log.h>
#ifndef FFMPEG_MY_LOG_H
#define FFMPEG_MY_LOG_H


#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"@Emery", FORMAT, ##__VA_ARGS__)
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"@Emery", FORMAT, ##__VA_ARGS__)


#endif //FFMPEG_MY_LOG_H
