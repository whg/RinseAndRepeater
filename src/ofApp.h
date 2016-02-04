#pragma once

#include "ofMain.h"

#include "ofxMidi.h"

#include "ofxFlexibleVideoPlayer.h"


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

    void audioOut( ofSoundBuffer& buffer );

    
    ofxMidiIn midiIn;
    void newMidiMessage(ofxMidiMessage& msg);

    ofxFlexibleVideoPlayer flexiPlayer;
    float starts[8];
    
    map<int, std::function<void(int)>> functionMap;
    map<int, std::function<void(int)>> startMap;

};
