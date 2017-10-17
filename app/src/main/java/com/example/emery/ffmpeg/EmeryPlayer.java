package com.example.emery.ffmpeg;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by emery on 2017/10/8.
 */

public class EmeryPlayer implements SurfaceHolder.Callback {
    private SurfaceView mSurfaceView;
    public EmeryPlayer(SurfaceView surfaceView){
        mSurfaceView = surfaceView;
        display(mSurfaceView.getHolder().getSurface());
        mSurfaceView.getHolder().addCallback(this);
    }



    public void play(String path){
        if(mSurfaceView==null){
            return;
        }
        audioVideoPlay(path);
    }

    public native void audioVideoPlay(String inputPath);
    public native void player();
    public native void stop();
    public native void release();
    public native void display(Surface surface);

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
       display(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}
