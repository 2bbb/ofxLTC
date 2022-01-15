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
        auto &target_device = list[3]; // select device id what you need to use
        ofSoundStreamSettings setting;
        setting.setInDevice(target_device);
        setting.numInputChannels = target_device.inputChannels;
        setting.sampleRate = target_device.sampleRates.front();
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
