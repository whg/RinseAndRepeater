#include "ofApp.h"

map<int, std::function<void(int)>> functionMap;
map<int, std::function<void(int)>> startMap;

#include "whelpersg/json/json.hpp"

void ofApp::setup() {
    
    std::ifstream f("../../../settings.json");
    auto json = nlohmann::json::parse(f);

    flexiPlayer.load(json["frames"], json["audio"]);
//    flexiPlayer.load("/Users/whg/Desktop/chris_sync", "/Users/whg/Desktop/chris_sync.wav");
    ofSoundStreamSetup(2, 0, 44100, 512, 2);
    
    ofxMidiIn::listPorts();
    string midiPort = json["midi-name"];
    midiIn.openPort(midiPort);
    midiIn.addListener(this);

    functionMap[3] = [this](int value) { this->flexiPlayer.setSpeed(ofMap(value, 0, 127, 0.1, 3)); };
    
//    float triggerFrames[] = { 2, 23, 60, 311, 352, 360, 367, 375 };
    vector<int> triggerFrames = json["start-points"];
    
    for (int i = 0; i < 8; i++) {
        starts[i] = triggerFrames[i];
        startMap[44+i] = [this, i](int vel) { this->flexiPlayer.setFrame(starts[i]); };
        
    }
    
}

void ofApp::audioOut( ofSoundBuffer& buffer ) {

    flexiPlayer.audioOut(buffer);
    
//    auto &data = buffer.getBuffer();
//    float lastSample;
//    int step = 10+1;
//    for (int i = 0; i < buffer.getNumFrames(); i++) {
//        if (i % step == 0) lastSample = data[i*2];
//        data[i*2] = data[i*2+1] = lastSample;
//    }
}

void ofApp::update() {
    
    flexiPlayer.update();
}


void ofApp::draw() {

    flexiPlayer.draw();
}


void ofApp::keyPressed(int key) {}
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}


void ofApp::newMidiMessage(ofxMidiMessage& msg) {
    
    if (msg.status == MIDI_CONTROL_CHANGE) {
        if (functionMap.count(msg.control)) {
            functionMap[msg.control](msg.value);
        }
    }
    else if (msg.status == MIDI_NOTE_ON) {
        if (startMap.count(msg.pitch)) {
            startMap[msg.pitch](msg.velocity);
        }
    }
}