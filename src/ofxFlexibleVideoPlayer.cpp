//
//  ofxFlexibleVideoPlayer.cpp
//  emptyExample
//
//  Created by Will Gallia on 04/02/2016.
//
//

#include "ofxFlexibleVideoPlayer.h"

ofxFlexibleVideoPlayer::ofxFlexibleVideoPlayer():
mLastUpdateTime(0),
mFrameRate(0),
mLoop(OF_LOOP_NORMAL),
mAudioStep(1) {
}

// framesFolder: a directory of images, in the right order, alphabetical?
// audioFile: a .wav file of the soundtrack
void ofxFlexibleVideoPlayer::load(string framesFolder, string audioFile, float frameRate) {
    
    ofDirectory dir(framesFolder);
    ofImage img;
    for (auto &file : dir.getFiles()) {
        auto path = file.getAbsolutePath();
        ofTexture tex;
        img.load(path);
        tex.loadData(img.getPixels());
        mTextures.emplace_back(std::move(tex));
    }
    
    ofLogNotice("ofxFlexibleVideoPlayer") << "loaded " << mTextures.size() << " images";
    
    mSoundtrackSample.load(audioFile);
    
    mAudioData.resize(mSoundtrackSample.length);
    for (int i = 0; i < mAudioData.size(); i++) {
        mAudioData[i] = mSoundtrackSample.temp[i] / 32767.0f;
    }
    
    mFrameRate = frameRate;
    mFrameTime = 1.0f / frameRate; // this should be from the frameRate of the video and not change
    mContentLength = int(mTextures.size()) * mFrameTime;
    
    /////////////////////////////////////
    mSoundtrackSample.getLength();
    auto audiolength = mSoundtrackSample.length;
    
    blendShader.load("shaders/blend");
    
//    mLastUpdateTime = 0;
//    mFrameTime = 0;
}

void ofxFlexibleVideoPlayer::update() {
    
    auto currentTime = ofGetElapsedTimef();
    auto timeSinceLastUpdate = currentTime - mLastUpdateTime;
    mLastUpdateTime = ofGetElapsedTimef();
    
    auto lastPlayhead = mPlayhead; // * 0.3 --- is pretty cool!!!
    mPlayhead+= timeSinceLastUpdate * 0.02;
    
    // simple loopback
    if (mLoop == OF_LOOP_NORMAL && mPlayhead >= mContentLength) {
        mPlayhead = 0;
        
        audioMutex.lock();
        mAudioPlayhead = 0;
        audioMutex.unlock();
    }
    
    audioMutex.lock();
//    mSoundtrackSample.setPosition(mPlayhead / mContentLength);
    mLastAudioPlayhead = mAudioPlayhead;
    auto playheadDiff = (mPlayhead - lastPlayhead);
    mAudioStep = playheadDiff / timeSinceLastUpdate;
    
//    if (ofGetKeyPressed('a')) {
//        mAudioPlayhead = (mPlayhead / mContentLength) * mAudioData.size();
//        printf("\n\n\n\n\n\n\n");
//    }
//    else if (ofGetKeyPressed('s')) {
//        cout << mAudioPlayhead - ((mPlayhead / mContentLength) * mAudioData.size()) << ",";
//    }
////    cout << "," << mAudioPlayhead;
//    if (mLastAudioPlayhead > mAudioPlayhead) {
//        mLastAudioPlayhead = mAudioPlayhead;
//    }
    audioMutex.unlock();
    
}

void ofxFlexibleVideoPlayer::draw() {
    
    float exactFrame = mPlayhead / mFrameTime;
    int frameA = int(floor(exactFrame));
    int frameB = int(ceil(exactFrame));
    float blend = exactFrame - frameA;
        
    assert(frameA >= 0 && frameA < mTextures.size());
    
    ofVec2f texSize(mTextures[0].getWidth(), mTextures[0].getHeight());
    ofMesh mesh = ofMesh::plane(texSize.x, texSize.y);
    
    mTextures[frameA].bind();
    
    ofTranslate(texSize / 2);
    ofEnableNormalizedTexCoords();
    blendShader.begin();

    blendShader.setUniformTexture("texA", mTextures[frameA], 0);
    blendShader.setUniformTexture("texB", mTextures[frameB], 1);
    blendShader.setUniform1f("blend", blend);
    blendShader.setUniform2f("size", texSize);
    mesh.drawFaces();
    
    blendShader.end();
    ofDisableNormalizedTexCoords();
    
    mTextures[frameA].unbind();
//    mTextures[frameA].draw(0, 0);
    
    ofDrawBitmapStringHighlight(ofToString(frameA), 10, 10);
    
    printf("%d, %d, %f\n", frameA, frameB, blend);
}

void ofxFlexibleVideoPlayer::audioOut(ofSoundBuffer& buffer) {
    
    ofScopedLock lock(audioMutex);
    
    auto &data = buffer.getBuffer();
//    std::copy(mAudioData.begin() + mAudioPlayhead, mAudioData.begin() + mAudioPlayhead + buffer.size(), data.begin());

//    float step = (mAudioPlayhead - mLastAudioPlayhead) / float(buffer.getNumFrames());
    
    auto mainDataLength = mAudioData.size();

    auto l = buffer.size() / 2;
    auto it = mAudioData.begin() + mAudioPlayhead;
    for (int i = 0; i < l; i++) {
        float pos = (mAudioPlayhead + mAudioStep * i);
        float a = mAudioData[int(floor(pos)) % mainDataLength];
        float b = mAudioData[int(ceil(pos)) % mainDataLength];
        float blend = pos - floor(pos);
        data[i*2] = data[i*2+1] = a * (1.f - blend) + b * blend;
//        data[i*2] = data[i*2+1] = mAudioData[(mAudioPlayhead + i) % mainDataLength];
//        ++it;
    }
    
    mAudioPlayhead+= l * mAudioStep;
    if (mAudioPlayhead >= mainDataLength) {
        mAudioPlayhead-= mainDataLength;
    }
}
