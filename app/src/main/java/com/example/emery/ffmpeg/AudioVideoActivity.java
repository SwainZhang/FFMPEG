package com.example.emery.ffmpeg;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

/**
 * Created by emery on 2017/10/8.
 */

public class AudioVideoActivity extends AppCompatActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_video);
    }

    public void audioVideoPlay(View view){
        EmeryPlayer emeryPlayer=new EmeryPlayer();
        emeryPlayer.audioVideoPlay(Constant.VIEDO_INPUT_PATH);
    }
}
