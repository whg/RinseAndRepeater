//
//  ofxFlexibleVideoPlayer.h
//  emptyExample
//
//  Created by Will Gallia on 04/02/2016.
//
//

#pragma once

#include "ofMain.h"
#include "ofxMaxim.h"

class ofxFlexibleVideoPlayer {
public:
    ofxFlexibleVideoPlayer();
    
    void load(string framesFolder, string audioFile, float frameRate=25);
    
    void update();
    void draw();
    
    void audioOut(ofSoundBuffer& buffer);
    
    void setPosition(float time);
    void setFrame(int frame);
    
protected:
    vector<ofTexture> mTextures;
    ofxMaxiSample mSoundtrackSample;
    vector<float> mAudioData;
    unsigned long mAudioPlayhead, mLastAudioPlayhead;
    float mAudioStep;
    
    float mFrameRate, mFrameTime;
    float mContentLength; // in seconds
    float mPlayhead; // playhead in seconds
protected:
    float mLastUpdateTime;
    
    ofLoopType mLoop;
    
    ofMutex audioMutex;
    
    ofShader blendShader;
};