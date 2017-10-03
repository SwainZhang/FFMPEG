package com.example.emery.ffmpeg;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;

/**
 * Created by emery on 2017/10/3.
 */

public class MyViedeoView extends SurfaceView {
    private static final String TAG  = "@Emery-"+MyViedeoView.class.getSimpleName();
    public MyViedeoView(Context context) {
        this(context,null);
    }

    public MyViedeoView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public MyViedeoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
      getHolder().setFormat(PixelFormat.RGBA_8888);
    }

    public void play(final String path){
        new Thread(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "run path="+path);
                render(path,getHolder().getSurface());
            }
        }).start();
    }

    public native void render(String path,Surface surface);
}
