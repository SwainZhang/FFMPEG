//
// Created by 张瑞 on 2017/10/12.
//

#ifndef FFMPEG_HEADER_H
#define FFMPEG_HEADER_H

#include "my-log.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include <pthread.h>
#include <queue>
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
}
#endif //FFMPEG_HEADER_H
