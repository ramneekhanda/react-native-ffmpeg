package com.rnffmpeg;

public interface IFFMpegCallback {

    void progress(int percentageDone);

    void done();

    void error(String err);
}
