
package com.reactlibrary;

import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.Callback;
import com.rnffmpeg.FFMpeg;
import com.facebook.react.bridge.Promise;
import com.rnffmpeg.IFFMpegCallback;


public class RNFFMpegModule extends ReactContextBaseJavaModule {


  private final ReactApplicationContext reactContext;


  public RNFFMpegModule(ReactApplicationContext reactContext) {
    super(reactContext);
    this.reactContext = reactContext;
  }

  @Override
  public String getName() {
    return "RNFFMpegModule";
  }

  @ReactMethod
  public void encodeVideo(String filename, String outputFilename, Callback progressCallback, Promise promise) {
    FFMpeg ffmpeg = new FFMpeg(new FFMpegCallback(promise, progressCallback));
    ffmpeg.encodeVideoOnly(filename, outputFilename);
  }
}

class FFMpegCallback implements IFFMpegCallback {
  private Promise promise = null;
  private Callback progressCallBack = null;

  FFMpegCallback(Promise promise, Callback progressCallBack) {
    this.promise = promise;
    this.progressCallBack = progressCallBack;
  }

  public void progress(int percentageDone) {
    progressCallBack.invoke(percentageDone);
  }

  public void done() {
    promise.resolve(true);
  }

  public void error(String err) {
    promise.reject(new Throwable(err));
  }
}
