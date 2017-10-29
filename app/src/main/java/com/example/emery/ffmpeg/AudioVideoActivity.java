package com.example.emery.ffmpeg;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;
import android.view.View;

/**
 * Created by emery on 2017/10/8.
 */

public class AudioVideoActivity extends AppCompatActivity {

    private SurfaceView mSurfaceView;
    private EmeryPlayer mEmeryPlayer;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_video);
        mSurfaceView = (SurfaceView) findViewById(R.id.surface);

    }

    public void audioVideoPlay(View view){
        mEmeryPlayer = new EmeryPlayer(mSurfaceView);
        mEmeryPlayer.play(Constant.VIEDO_INPUT_PATH);
      //  emeryPlayer.play("rtmp://live.hkstv.hk.lxdns.com/live/hks");

    }

    @Override
    protected void onStop() {
        super.onDestroy();
        mEmeryPlayer.release();
    }
}
