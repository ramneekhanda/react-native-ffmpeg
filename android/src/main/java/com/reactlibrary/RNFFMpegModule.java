
package com.reactlibrary;

import android.support.annotation.Nullable;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.ReadableMap;
import com.rnffmpeg.FFMpeg;
import com.facebook.react.bridge.Promise;
import com.rnffmpeg.IFFMpegCallback;
import com.facebook.react.modules.core.RCTNativeAppEventEmitter;
import com.facebook.react.bridge.WritableMap;

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
  public void encodeVideoOnly(final ReadableMap options, final Promise promise) {
    int jobId = options.getInt("jobId");
    String fromFile = options.getString("fromFile");
    String toFile = options.getString("toFile");
    FFMpeg ffmpeg = new FFMpeg(new FFMpegCallback(jobId, promise, reactContext));
    ffmpeg.encodeVideoOnly(fromFile, toFile);
  }
}

class FFMpegCallback implements IFFMpegCallback {
  private final ReactApplicationContext reactContext;
  private Promise promise = null;
  private int jobId;

  FFMpegCallback(int jobId, Promise promise, ReactApplicationContext reactContext) {
    this.jobId = jobId;
    this.reactContext = reactContext;
    this.promise = promise;
  }

  private void sendEvent(ReactContext reactContext, String eventName, @Nullable WritableMap params) {
    reactContext
            .getJSModule(RCTNativeAppEventEmitter.class)
            .emit("RNFFMPEG-PROGRESS-" + jobId, params);
  }

  public void progress(int percentageDone) {
    System.out.println("progressCallback not defined - percentage completed - " + percentageDone);
    WritableMap params = Arguments.createMap();
    params.putInt("jobId", this.jobId);
    params.putInt("progress", percentageDone);
    sendEvent(reactContext, "Progress", params);
  }

  public void done() {
    if (promise != null)
      promise.resolve(true);
  }

  public void error(String err) {
    if (promise != null)
      promise.reject(new Throwable(err));
  }
}
