package com.example.emery.ffmpeg;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

/**
 * Created by emery on 2017/10/3.
 */

public class VideoPlayActivity extends AppCompatActivity {

    private MyViedeoView mVideoView;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_play);
        mVideoView = (MyViedeoView) findViewById(R.id.videoView);
    }

    public void convertYuv(View view){
        new Thread(new Runnable() {
            @Override
            public void run() {
             VideoPlayer videoPlayer=new VideoPlayer();
                videoPlayer.convertYuv(Constant.VIEDO_INPUT_PATH, Constant.VIEDO_OUT_PATH);
            }
        }).start();
    }

    public void onlyPlayVideo(View view){
        mVideoView.play(Constant.VIEDO_INPUT_PATH);
    }


}
