'use strict';

import { NativeModules } from 'react-native';
var ReactNativeFFMpegModule = require('react-native').NativeModules.RNFFMpegModule;

var NativeAppEventEmitter = require('react-native').NativeAppEventEmitter;        
var DeviceEventEmitter = require('react-native').DeviceEventEmitter;        // Android

var jobId = 0;

var getJobId = () => {
  jobId += 1;
  return jobId;
};

type ProgressCallbackResult = {
  jobId: number;          
  progress: number;
};

type EncodeOptions = {
  fromFile: string;          
  toFile: string;           
  progress?: (res: ProgressCallbackResult) => void;
};

var ReactNativeFFMpeg = {
  encodeVideoOnly(options: EncodeOptions): { jobId: number, promise: Promise<ProgressCallbackResult> } {
    if (typeof options !== 'object') throw new Error('encodeVideoOnly: Invalid value for argument `options`');
    if (typeof options.fromFile !== 'string') throw new Error('encodeVideoOnly: Invalid value for property `fromFile`');
    if (typeof options.toFile !== 'string') throw new Error('encodeVideoOnly: Invalid value for property `toFile`');

    var jobId = getJobId();
    var subscriptions = [];

    if (options.progress) {
      subscriptions.push(NativeAppEventEmitter.addListener('RNFFMPEG-PROGRESS-' + jobId, options.progress));
    }

    var bridgeOptions = {
      jobId: jobId,
      fromFile: options.fromFile,
      toFile: options.toFile
    };

    return {
      jobId,
      promise: ReactNativeFFMpegModule.encodeVideoOnly(bridgeOptions).then(res => {
        subscriptions.forEach(sub => sub.remove());
        return res;
      })
    };
  }
}

module.exports = ReactNativeFFMpeg;