#include "ofApp.h"

#define NO_INDEX_SET -1
#define MIDI_NOTE_OUT_OFFSET 60
#define MIDI_NOTE_OUT_CHANNEL 11

map<int, std::function<void(int)>> functionMap;
map<int, std::function<void(int)>> startMap;

#include "whelpersg/json/json.hpp"

#include "whelpersg/ofxTools.h"

#define DEBUG

const char *hitKeys = "asdfghjk";

static const string vertShader = GLSL150(
    uniform mat4 modelViewProjectionMatrix;

    in vec4  position;
    in vec2  texcoord;

    out vec2 texCoordVarying;

    void main() {
       texCoordVarying = (vec4(texcoord.x,texcoord.y,0,1)).xy;
       gl_Position = modelViewProjectionMatrix * position;
    }
);


static const string fragShader = GLSL150(
    uniform sampler2DRect tex;
    uniform vec2 size;
    uniform float resolutionCrush;
    uniform float rateCrush;
                                                           
    in vec2 texCoordVarying;

    out vec4 fragColor;

    void main(){
        //make the mosaic pixels square
        vec2 squarePixel = vec2(1.0, size.x / size.y);
        
        vec2 scaledCoord = texCoordVarying * size;// * squarePixel;
        vec2 centeredCoord = (scaledCoord) - mod(scaledCoord-size/2, vec2(rateCrush));
        vec4 rateCrushCol = texture(tex, centeredCoord);
        vec3 resolutionCrushCol = floor(rateCrushCol.rgb * resolutionCrush) / resolutionCrush;
        
        fragColor = vec4(resolutionCrushCol, rateCrushCol.a);
        
    }
);

void ofApp::setup() {

    ofDrawBitmapString("Loading images...", 10, 10);
        
#ifdef DEBUG

    std::ifstream f("../../../settings.json");
    auto json = nlohmann::json::parse(f);

    flexiPlayer.load(json["frames"], json["audio"]);
#else
    
    auto imageFolderResult = ofSystemLoadDialog("Choose the images folder", true);
    
    string soundtrackPath = "";
    bool exit = false;
    while (soundtrackPath.find(".wav") == -1) {
        auto soundtrackFolderResult = ofSystemLoadDialog("Choose the soundtrack (it needs to be a .wav)", false);
        soundtrackPath = soundtrackFolderResult.getPath();
        
        if (!soundtrackFolderResult.bSuccess) {
            exit = true;
            break;
        }
    }
    if (exit) {
        ofExit();
    }
    else {
        flexiPlayer.load(imageFolderResult.getPath(), soundtrackPath);
    }

#endif
    
    panel.setup();

    speed.addListener(this, &ofApp::speedChanged);
    panel.add(speed.set("speed", 1, -2, 2));
    panel.add(resolutionCrush.set("resolutionCrush", 100, 100, 1));
    panel.add(rateCrush.set("rateCrush", 1, 1, 200));

    
    midiInputDevices.setName("MIDI Input");
    auto &devices = ofxMidiIn::getPortList();
    for (const auto &deviceName : devices) {
        midiInputDevices.addChoice(deviceName);
    }
    ofAddListener(midiInputDevices.changeEvent, this, &ofApp::midiInputDeviceChange);
    panel.add(midiInputDevices);
	midiIn.openPort(midiInputDevices.getCurrentChoice());
	midiIn.addListener(this);
	
	midiOutputDevices.setName("MIDI Output");
	auto &outputDevices = ofxMidiOut::getPortList();
	for (const auto &deviceName : outputDevices) {
		midiOutputDevices.addChoice(deviceName);
	}
	ofAddListener(midiOutputDevices.changeEvent, this, &ofApp::midiOutputDeviceChange);
	panel.add(midiOutputDevices);
    midiOut.openPort(midiOutputDevices.getCurrentChoice());

	lastCuePlayedIndex = NO_INDEX_SET;

    vector<int> triggerFrames = { 2, 23, 60, 311, 352, 360, 367, 375 };
    startFrames.resize(8);
    startNotes.resize(8);
    startGroups.resize(8);
    loopLengths.resize(8);
    
    startsPanel.setup("Starts", "starts.xml", 300, 10);
    
    for (int i = 0; i < 8; i++) {
        startGroups[i].setup("Start " + ofToString(i+1));

        startGroups[i].add(startFrames[i].set("Frame", triggerFrames[i], 0, flexiPlayer.getNumFrames()));
        startGroups[i].add(startNotes[i].set("MIDI note (key " + ofToString(hitKeys[i]) +")", 44+i, 0, 127));
        startGroups[i].add(loopLengths[i].set("Loop length", 1, 0, flexiPlayer.getNumFrames()));
        startsPanel.add(&startGroups[i]);

    }
    startVideoAtCueIndex(0);
    doLoop = false;
    
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
    mosaicShader.setupShaderFromSource(GL_VERTEX_SHADER, vertShader);
    mosaicShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShader);
    mosaicShader.bindDefaults();
    mosaicShader.linkProgram();
    
    maxCrush = 320;
    
    midiMapper.setup(midiIn);

    ofSetDataPathRoot("../Resources/");
    logoImage.load("itg_logo128.png");
    
    ofSetWindowTitle("Rinse and Repeater");
//    ofSoundStreamSetup(2, 0);
}

void ofApp::audioOut( ofSoundBuffer& buffer ) {

//    flexiPlayer.audioOut(buffer);
//    
//    auto &data = buffer.getBuffer();
//    float lastSample, v;
//    for (int i = 0; i < buffer.getNumFrames(); i++) {
//        if (i % rateCrush == 0) v = data[i*2];
//
//        v = int(v * resolutionCrush) / float(resolutionCrush);
//        
//        data[i*2] = data[i*2+1] = v;
//    }
}

void ofApp::update() {

    
    flexiPlayer.update();

    
    if (doLoop) {
        auto loopEndFrame = startFrames[currentCueIndex] + loopLengths[currentCueIndex];
        
        if (flexiPlayer.getFrame() > loopEndFrame) {
            startVideoAtCueIndex(currentCueIndex);
        }
    }

}


void ofApp::draw() {
    
    fbo.begin();
    flexiPlayer.draw();
    fbo.end();
    
    
    ofVec2f size(ofGetWidth(), ofGetHeight());
    ofMesh mesh = ofMesh::plane(size.x, size.y);
    
    ofPushMatrix();
    ofTranslate(size / 2);
    
    mosaicShader.begin();
    mosaicShader.setUniformTexture("tex", fbo.getTexture(), 0);
    mosaicShader.setUniform2f("size", size);
    
    mosaicShader.setUniform1f("resolutionCrush", float(resolutionCrush/2));
    mosaicShader.setUniform1f("rateCrush", float(rateCrush));

    mesh.drawFaces();
    
    mosaicShader.end();
    ofPopMatrix();
    
    logoImage.draw(ofGetWidth()-logoImage.getWidth()-5, ofGetHeight()-logoImage.getHeight()-5);
    
    if (panel.isVisible()) {
        panel.draw();
        startsPanel.draw();
    }
}

void ofApp::startVideoAtCueIndex(int index) {
    flexiPlayer.setFrame(startFrames[index]);
    flexiPlayer.setCueFrame(startFrames[index]);
    currentCueIndex = index;
    doLoop = true;
	
	if (lastCuePlayedIndex != NO_INDEX_SET) {
		midiOut.sendNoteOff(MIDI_NOTE_OUT_CHANNEL, MIDI_NOTE_OUT_OFFSET + lastCuePlayedIndex);
	}
	
    midiOut.sendNoteOn(MIDI_NOTE_OUT_CHANNEL, MIDI_NOTE_OUT_OFFSET + index, 127);
	
	lastCuePlayedIndex = index;
}


void ofApp::keyPressed(int key) {

    KEY('r', flexiPlayer.setSpeed(flexiPlayer.getSpeed() * -1));
    KEY(' ', panel.toggleVisibility());
    
    if (doLoop) return; // don't retrigger from keyPress being called
    
    for (int i = 0; i < startFrames.size(); i++) {
        KEY(hitKeys[i], {
            startVideoAtCueIndex(i);
            break;
        });

    }
	
	currentKeyDown = key;
}

void ofApp::keyReleased(int key) {
    for (int i = 0; i < startFrames.size(); i++) {
        KEY(hitKeys[i], doLoop = false);
    }
}
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
        for (int i = 0; i < startNotes.size(); i++) {
            if (msg.pitch == startNotes[i]) {
                startVideoAtCueIndex(i);
                break;
            }
        }
    }
    else if (msg.status == MIDI_NOTE_OFF) {
        doLoop = false;
    }
}

void ofApp::midiInputDeviceChange(ofxRadioGroupEventArgs &args) {
    midiIn.closePort();
    midiIn.openPort(args.name);
	}

void ofApp::midiOutputDeviceChange(ofxRadioGroupEventArgs &args) {
	
	midiOut.closePort();
	midiOut.openPort(args.name);
}