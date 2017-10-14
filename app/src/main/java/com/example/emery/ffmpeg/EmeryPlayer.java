package com.example.emery.ffmpeg;

import android.view.Surface;

/**
 * Created by emery on 2017/10/8.
 */

public class EmeryPlayer {
    public native void audioVideoPlay(String inputPath);
    public native void player();
    public native void stop();
    public native void release();
    public native void display(Surface surface);
}
