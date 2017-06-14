package com.rnffmpeg;

public class FFMpeg {
    static {
        System.loadLibrary("ffmpeg_wrap");
    }
    public native static int encodeVideo(String filename, String outputfile);
}
