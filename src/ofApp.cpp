#include "ofApp.h"

map<int, std::function<void(int)>> functionMap;
map<int, std::function<void(int)>> startMap;

#include "whelpersg/json/json.hpp"


#define STRINGIFY(x) #x

static const string shaderVersion = "#version 150\n";
static const string vertShader = shaderVersion + STRINGIFY(
    uniform mat4 modelViewProjectionMatrix;

    in vec4  position;
    in vec2  texcoord;

    out vec2 texCoordVarying;

    void main() {
       texCoordVarying = (vec4(texcoord.x,texcoord.y,0,1)).xy;
       gl_Position = modelViewProjectionMatrix * position;
    }
);


static const string fragShader = shaderVersion + STRINGIFY(
    uniform sampler2DRect tex;
    uniform vec2 size;
    uniform float resolutionCrush;
    uniform float rateCrush;
                                                           
    in vec2 texCoordVarying;

    out vec4 fragColor;

    void main(){
        //make the mosaic pixels square
        vec2 crushVector = vec2(rateCrush) * vec2(1.0, size.y / size.x);
        
        vec4 rateCrushCol = texture(tex, (floor(texCoordVarying * crushVector) / crushVector) * size);
        vec3 resolutionCrushCol = floor(rateCrushCol.rgb * resolutionCrush) / resolutionCrush;
        
        fragColor = vec4(resolutionCrushCol, rateCrushCol.a);
        
    }
);

void ofApp::setup() {
    
    std::ifstream f("../../../settings.json");
    auto json = nlohmann::json::parse(f);

    flexiPlayer.load(json["frames"], json["audio"]);
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
    
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
    mosaicShader.setupShaderFromSource(GL_VERTEX_SHADER, vertShader);
    mosaicShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShader);
    mosaicShader.bindDefaults();
    mosaicShader.linkProgram();
    
    maxCrush = 320;
    
    panel.setup();
    panel.add(resolutionCrush.set("resolutionCrush", 200, 200, 1));
    panel.add(rateCrush.set("rateCrush", 1, 1, 200));

}

void ofApp::audioOut( ofSoundBuffer& buffer ) {

    flexiPlayer.audioOut(buffer);
    
    auto &data = buffer.getBuffer();
    float lastSample, v;
    for (int i = 0; i < buffer.getNumFrames(); i++) {
        if (i % rateCrush == 0) v = data[i*2];

        v = int(v * resolutionCrush) / float(resolutionCrush);
        
        data[i*2] = data[i*2+1] = v;
    }
}

void ofApp::update() {
    
    flexiPlayer.update();
    
//    crushAmount = MIN(maxCrush, MAX(1, mouseX));

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
    
    mosaicShader.setUniform1f("resolutionCrush", float(resolutionCrush));
    mosaicShader.setUniform1f("rateCrush", float(rateCrush.getMax() - rateCrush));

    mesh.drawFaces();
    
    mosaicShader.end();
    ofPopMatrix();
    
    panel.draw();
}


void ofApp::keyPressed(int key) {
    if (key == ' ') {
        flexiPlayer.setSpeed(flexiPlayer.getSpeed() * -1);
    }
}

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