package com.rnffmpeg;

public class FFMpeg {
    public native static int encodeVideo(String filename, String outputfile, int codecType);
}
