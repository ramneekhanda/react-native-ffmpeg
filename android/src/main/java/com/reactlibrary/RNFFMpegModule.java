
package com.reactlibrary;

import android.support.annotation.Nullable;
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
  public void encodeVideo(final ReadableMap options, final Promise promise) {
    FFMpeg ffmpeg = new FFMpeg(new FFMpegCallback(promise, reactContext));
    ffmpeg.encodeVideoOnly(filename, outputFilename);
  }
}

class FFMpegCallback implements IFFMpegCallback {
  private final ReactApplicationContext reactContext;
  private Promise promise = null;

  FFMpegCallback(Promise promise, ReactApplicationContext reactContext) {
    this.reactContext = reactContext;
    this.promise = promise;
  }

  private void sendEvent(ReactContext reactContext, String eventName, @Nullable WritableMap params) {
    reactContext
            .getJSModule(RCTNativeAppEventEmitter.class)
            .emit(eventName, params);
  }

  public void progress(int percentageDone) {
    System.out.println("progressCallback not defined - percentage completed - " + percentageDone);
    sendEvent(reactContext, "Progress", null);
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
