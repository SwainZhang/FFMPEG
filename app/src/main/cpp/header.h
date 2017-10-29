//
// Created by 张瑞 on 2017/10/12.
//

#ifndef FFMPEG_HEADER_H
#define FFMPEG_HEADER_H

#include "my-log.h"
#include <android/native_window_jni.h>

#include <queue>
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"

extern "C" {
#include <pthread.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/time.h"
}
#endif //FFMPEG_HEADER_H
