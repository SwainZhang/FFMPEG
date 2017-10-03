package com.example.emery.ffmpeg;

import android.os.Environment;

import java.io.File;

/**
 * Created by emery on 2017/10/3.
 */

public class Constant {
    public static final String VIEDO_INPUT_PATH= Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator+"MyMultiThreadDownload"+File.separator+"input.mp4";
    public static final String VIEDO_OUT_PATH= Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator+"MyMultiThreadDownload"+File.separator+"output.yuv";

    public static final String AUDIO_INPUT_PATH=Environment.getExternalStorageDirectory().getAbsolutePath()+File.separator+"MyMultiThreadDownload"+File.separator+"music.mp3";
    public static final String AUDIO_OUT_PATH=Environment.getExternalStorageDirectory().getAbsolutePath()+File.separator+"MyMultiThreadDownload"+File.separator+"music.pcm";

}
