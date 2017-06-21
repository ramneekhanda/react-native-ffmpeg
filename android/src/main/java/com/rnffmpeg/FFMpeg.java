package com.rnffmpeg;

public class FFMpeg {
    IFFMpegCallback ffmpegCallback = null;

    static {
        System.loadLibrary("ffmpeg_wrap");
    }

    public FFMpeg(IFFMpegCallback ffmpegCallback) {
        this.ffmpegCallback = ffmpegCallback;
    }

    void progress(int percentageDone) {
        ffmpegCallback.progress(percentageDone);
    }

    void done() {
        ffmpegCallback.done();
    }

    void error() {
        ffmpegCallback.error("Error Occurred");
    }

    public native void encodeVideoOnly(String filename, String outputFile);
}
