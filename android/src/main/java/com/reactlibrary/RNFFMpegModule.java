
package com.reactlibrary;

import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.Callback;
import com.rnffmpeg.FFMpeg;

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
  public int encodeVideo(String filename, String outputFilename) {
    FFMpeg ffmpeg = new FFMpeg();
    return ffmpeg.encodeVideo(filename, outputFilename);
  }
}