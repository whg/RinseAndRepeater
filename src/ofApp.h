#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "ofxMidi.h"

class ofApp : public ofBaseApp, public ofxMidiListener {
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

    ofVideoPlayer player;
    
    ofxPanel panel;
    bool showGui;
    
    ofParameter<float> position;
    ofParameter<float> start;
    ofParameter<float> duration;
    ofParameter<int> frameRate;
    ofParameter<bool> reverse;
    vector<ofImage> texes;
    
    int framePosition;
    float minDuration;
    
    ofxMidiIn midiIn;
    void newMidiMessage(ofxMidiMessage& msg);
    
    ofxMidiOut midiOut;
};

#include "ofxSlider.h"

template<typename T>
class ofxMidiMappableSlider : public ofxSlider<T> {
    
    virtual bool mousePressed(ofMouseEventArgs & args) override {
//        cout << "hello" << end;
        ofxSlider<T>::mousePressed(args);
    }
};
