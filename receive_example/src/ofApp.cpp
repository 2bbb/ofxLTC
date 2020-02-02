#include "ofxLTC.h"
#include "ofMain.h"

class ofApp : public ofBaseApp {
    ofSoundBuffer buffer;
    ofSoundPlayer player;
    ofxLTCReceiver receiver;
    ofxLTCTimecode timecode;
    ofThreadChannel<ofxLTCTimecode> received_timecodes;
public:
    inline virtual void setup() override {
        auto &&list = receiver.getDeivceList();
        for(auto i = 0; i < list.size(); ++i) {
            const auto &device = list[i];
            ofLogNotice() << i << ": " << device.name << " " << device.inputChannels;
        }
        ofSoundStreamSettings setting;
        setting.setInDevice(list[0]);
        receiver.setup(setting);
        receiver.onReceive([=](ofxLTCTimecode code) {
            received_timecodes.send(code);
        });
    }
    inline virtual void update() override {
        while(!received_timecodes.empty()) {
            received_timecodes.receive(timecode);
        }
    }
    inline virtual void draw() override {
        ofBackground(20);
        ofSetColor(255, 0, 0);
        ofDrawBitmapString(timecode.toString(), 20, 20);
    }
};

int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    ofRunApp(new ofApp());
}
