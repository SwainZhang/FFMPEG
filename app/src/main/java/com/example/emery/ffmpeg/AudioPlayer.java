package com.example.emery.ffmpeg;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

/**
 * Created by emery on 2017/10/3.
 */

public class AudioPlayer {
    private static final String TAG  = "@Emery-"+AudioPlayer.class.getSimpleName();
    private AudioTrack mAudioTrack;

    public native void convertPCM(String inputPath, String outputPath);
    public native void directPlay(String inputPath);


    public void playPCM(String inputString){
        int minBufferSize = AudioTrack.getMinBufferSize(44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT);
        mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,44100,AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT,minBufferSize,AudioTrack.MODE_STREAM);
        mAudioTrack.play();
        try {
            FileInputStream fileInputStream=new FileInputStream(inputString);

            int length=-1;
            byte[] buffer=new byte[44100*2];
           while ((length=fileInputStream.read(buffer))!=-1){
               mAudioTrack.write(buffer,0,length);
           }


        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public void createAudioTrack(int nb_channels){
        Log.i(TAG, "createAudioTrack nb_channels="+nb_channels);
        int channelConfig;
        if(nb_channels==1){
          channelConfig=  AudioFormat.CHANNEL_OUT_MONO;
        }else if(nb_channels==2){
            channelConfig=AudioFormat.CHANNEL_OUT_STEREO;
        }else{
            channelConfig=  AudioFormat.CHANNEL_OUT_MONO;
        }
        int minBufferSize = AudioTrack.getMinBufferSize(44100, channelConfig, AudioFormat.ENCODING_PCM_16BIT);
        mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,44100,channelConfig, AudioFormat.ENCODING_PCM_16BIT,minBufferSize,AudioTrack.MODE_STREAM);
        mAudioTrack.play();
        Log.i(TAG, "createAudioTrack ");
    }

    public void writeByte(byte[] data,int length){

        if(mAudioTrack!=null&&mAudioTrack.getPlayState()==AudioTrack.PLAYSTATE_PLAYING){
            Log.i(TAG, "writeByte ");
            mAudioTrack.write(data,0,length);
        }
    }
}
