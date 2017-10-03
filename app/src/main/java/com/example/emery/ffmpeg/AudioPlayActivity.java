package com.example.emery.ffmpeg;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import java.io.File;

/**
 * Created by emery on 2017/10/3.
 */

public class AudioPlayActivity extends AppCompatActivity {
    private static final String TAG  = "@Emery-"+AudioPlayActivity.class.getSimpleName();
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_play);

    }
    public void convertPCM(View view){
        new Thread(new Runnable() {
            @Override
            public void run() {
             AudioPlayer audioPlayer=new AudioPlayer();
                File file=new File(Constant.AUDIO_INPUT_PATH);
                if(file.exists()){
                    Log.i(TAG, "run file exists-"+file.getAbsolutePath());
                }
                audioPlayer.convertPCM(file.getAbsolutePath(),Constant.AUDIO_OUT_PATH);
            }
        }).start();
    }

    public void directPlay(View v){
        new Thread(new Runnable() {
            @Override
            public void run() {
                AudioPlayer audioPlayer=new AudioPlayer();
                audioPlayer.directPlay(Constant.AUDIO_INPUT_PATH);
            }
        }).start();
    }

    public void playPCM(View v){
        new Thread(new Runnable() {
            @Override
            public void run() {
                AudioPlayer audioPlayer=new AudioPlayer();
                audioPlayer.playPCM(Constant.AUDIO_OUT_PATH);
            }
        }).start();
    }

}
