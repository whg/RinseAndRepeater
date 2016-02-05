#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxMidi.h"
#include "whelpersg/ofxRadioGroup.h"

#include "ofxFlexibleVideoPlayer.h"
#include "whelpersg/ofxMidiMapper.h"


class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);

    void audioOut( ofSoundBuffer& buffer );

    ofxPanel panel;
    bool showGui;
    ofxRadioGroup midiDevices;
    void midiDeviceChange(ofxRadioGroupEventArgs &args);
    
    ofxMidiIn midiIn;
    void newMidiMessage(ofxMidiMessage& msg);

    ofxFlexibleVideoPlayer flexiPlayer;

    ofParameter<float> speed;
    void speedChanged(float &speed) { flexiPlayer.setSpeed(speed); }

    vector<ofxGuiGroup> startGroups;
    vector<ofParameter<int>> startFrames;
    vector<ofParameter<int>> startNotes;

    
    map<int, std::function<void(int)>> functionMap;
    map<int, std::function<void(int)>> startMap;

    ofFbo fbo;
    ofShader mosaicShader;
    ofParameter<int> resolutionCrush, rateCrush;
    int crushAmount, maxCrush;
    
    ofxMidiMapper midiMapper;
    
    ofParameter<float> something;
};
