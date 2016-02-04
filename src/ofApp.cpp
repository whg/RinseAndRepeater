#include "ofApp.h"


map<int, std::function<void(int)>> functionMap;

void ofApp::setup() {

    ofBackground(0);

    player.load("test_move_short.mov");
    player.play();
    
    panel.setup("params");
    panel.add(position.set("position", 0, 0, 1));
    panel.add(start.set("start", 0, 0, 1));
    panel.add(duration.set("duration", 0.5, 0, 1));
    panel.add(frameRate.set("frameRate", 30, 15, 240));
    panel.add(reverse.set("reverse", false));

    
    framePosition = 0;
    
    ofxMidiIn::listPorts();
    midiIn.openPort(2);
    midiIn.addListener(this);
    
    midiOut.openPort(0);
    
    
    
    auto func = [](int value) { cout << "value from lambda " << value << endl; };
    
    functionMap[1] = [this](int value) { this->start = value/127.0; this->framePosition = this->start * this->texes.size(); };
    functionMap[2] = [this](int value) { this->duration = value/127.0 + this->minDuration; };
    functionMap[3] = [this](int value) { this->frameRate = ofMap(value, 0, 127, 15, 240); };



    playHeadTime = 0;
//
//
//    cout << player.getTotalNumFrames() << endl;
}


void ofApp::update() {


    //cout << player.isInitialized() << endl;
    
    if (player.isInitialized() && texes.size() == 0) {
        player.firstFrame();
        texes.resize(player.getTotalNumFrames());
        for (int i = 0; i < player.getTotalNumFrames(); i++) {
            auto &img = texes[i];
            img.setFromPixels(player.getPixels());
    //        texes.push_back(player.getTexture());
            player.nextFrame();
            player.update();
            ofSleepMillis(50);
            cout << "added frame " << i << endl;
        }
        
        //minDuration = (1.0f / int(texes.size())) * 1.0;
    }
    

    
    static float lastFrameTime = ofGetElapsedTimef();
    static float timeSinceLastFrame = 0;
    float timedelta = ofGetElapsedTimef() - lastFrameTime;
    lastFrameTime = ofGetElapsedTimef();

    
    timeSinceLastFrame+= timedelta;
    float frameTime = 1.0f / frameRate;
    
    printf("%f, %f, %d, %f\n", timedelta, timeSinceLastFrame, framePosition, frameTime);
    
    if (timeSinceLastFrame >= frameTime) {
        int framesMoved = timeSinceLastFrame / frameTime;
        framePosition+= reverse ? -framesMoved : framesMoved;
        
        float timeMoved = framesMoved * frameTime;
        timeSinceLastFrame-= timeMoved;
        
        playHeadTime+= reverse ? -timeMoved : timeMoved;
    }
    
    auto durationInFrames = duration * int(texes.size());
//    durationInFrames+= 1;
    
    if (reverse && framePosition < int(start * texes.size())) {
        framePosition = int(start*texes.size()-1) + durationInFrames;
    }
    else if (!reverse && framePosition >= start*int(texes.size()) + durationInFrames) {
        framePosition = int(start*texes.size());
//        midiOut.sendNoteOn(1, 48, 127);
    }
    
    if (framePosition >= int(texes.size())) {
        framePosition = 0;
    }
    else if (framePosition < 0) {
        //        framePosition = texes.size() - 1;
        framePosition = int((start)*texes.size()) + durationInFrames;
    }
    
    position = framePosition/float(texes.size());
    
    
//    cout << framePosition << endl;
    
//    position = player.getPosition();
//    
//    if (position >= start + duration) {
//        player.setPosition(start);
//        cout << "set to " << start << endl;
//    }
//    
//    player.update();

}


void ofApp::draw() {
//    player.draw(0, 0);
//    


    if(player.isInitialized()) {
//        int index = ofClamp(mouseX, 0, texes.size()-1);
        //int index = ofMap(position, 0, 1, 0, texes.size()-1);
        if (framePosition >= 0 && framePosition < texes.size()) {
            float playerRatio = player.getHeight() / float(player.getWidth());
            auto height = ofGetWidth() * playerRatio;
            auto yOffset = (ofGetHeight() - height) / 2.0;
//            auto screenRatio = ofGetWidth() / float(ofGetHeight());
            
            texes[framePosition].draw(0, yOffset, ofGetWidth(), height);
        }
    }
    else {
        ofDrawBitmapString("loading images...", 50, 50);
    }
    
    if (showGui) panel.draw();
    
    ofSetColor(ofColor::white);
    ofDrawCircle(mouseX, mouseY, 2);
}


void ofApp::keyPressed(int key) {

    player.setPosition(0);
    
    if (key == ' ') {
        showGui = !showGui;
    }
}


void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}


void ofApp::newMidiMessage(ofxMidiMessage& msg) {
//    cout << msg.control << endl;
    
    if (functionMap.count(msg.control)) {
        functionMap[msg.control](msg.value);
    }
}