/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 * @flow
 */

import React, { Component } from 'react';
import {
  AppRegistry,
  StyleSheet,
  Text,
  View,
  TouchableHighlight,
  Icon,
  Image,
  Button
} from 'react-native';

import ReactNativeFFMpeg from 'react-native-ffmpeg';
import resolveAssetSource from 'react-native/Libraries/Image/resolveAssetSource';
import RNFS from 'react-native-fs';

export default class test_app extends Component {
  
  constructor(props) {
    super(props);
    this.state = {
      fileName: "",
      imageAvailable: false
    }
  }

  getRemoteFileName(url) {
    //this removes the anchor at the end, if there is one
    url = url.substring(0, (url.indexOf("#") == -1) ? url.length : url.indexOf("#"));
    //this removes the query after the file name, if there is one
    url = url.substring(0, (url.indexOf("?") == -1) ? url.length : url.indexOf("?"));
    //this removes everything before the last slash in the path
    url = url.substring(url.lastIndexOf("/") + 1, url.length);
    //return
    return url;
  }

  onConvert() {
    var localInputFileName = "";
    var randomFileName = RNFS.CachesDirectoryPath + "/" + Math.random().toString(36).substring(5) + ".gif";
    var resolvedFile = localInputFileName = "http://techslides.com/demos/sample-videos/small.mp4";

    this.setState({
      fileName: randomFileName,
      imageAvailable: false
    });

    console.log("resolved file " + resolvedFile);
    console.log("writing output file " + randomFileName);
    var remoteFileName = this.getRemoteFileName(resolvedFile);
    const progress = p => { 
      console.log("javascript completed percent - " + p.progress); 
    }
    if (resolvedFile.search(/http/i) >= 0) {
      var localInputFileName = RNFS.CachesDirectoryPath + "/" + remoteFileName;
      console.log("remote file is being used - probably running under debugger. Will download the file now");
      console.log("Downloading to file " + localInputFileName);
      RNFS.downloadFile({
        fromUrl: resolvedFile,
        toFile: localInputFileName
      }).promise.then(res => {
        console.log("Downloaded file file://" + localInputFileName);
        this.setState({
          imageAvailable: false
        });
        ReactNativeFFMpeg.encodeVideoOnly(
          {
            fromFile: localInputFileName,
            toFile: randomFileName,
            progress
          }
        ).promise.then(() => {
          console.log("completed conversion");
          this.setState({
            imageAvailable: true
          });
        })
          .catch((err) => { console.log("Error occurred: " + err) });
      }).catch(err => {
        console.log("Failed to download file")
      })
    }
  }

  render() {
    var ImageComponent = <View />
    if (this.state.imageAvailable) {
      console.log("showing image " + this.state.fileName)

      ImageComponent = <View style={styles.preview}><Image
        style={{ width: 300, height: 300}}
        source={{ uri: "file://" + this.state.fileName }}
        resizeMode="contain"
      /></View>
    }
    return (
      <View style={styles.container}>
      <Button
        onPress={this.onConvert.bind(this)}
        title="Convert Video to GIF"
        color="#841584"
      />
        {ImageComponent}
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1
  },
  preview: {
    flex: 1,
    justifyContent: 'flex-start',
    alignItems: 'center'
  },
  capture: {
    flex: 0,
    backgroundColor: '#fff',
    borderRadius: 5,
    color: '#000',
    padding: 10,
    margin: 40
  },
  welcome: {
    fontSize: 20,
    textAlign: 'center',
    margin: 10,
  },
  instructions: {
    textAlign: 'center',
    color: '#333333',
    marginBottom: 5,
  },
});

AppRegistry.registerComponent('test_app', () => test_app);
