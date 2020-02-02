//
//  ofxLTC.h
//
//  Created by 2bit on 2020/02/02.
//

#ifndef ofxLTC_h
#define ofxLTC_h

#include "decoder.h"
#include "ofSoundStream.h"
#include "ofSoundBuffer.h"
#include "ofUtils.h"

namespace ofx {
    namespace LTC {
        struct Timecode {
            LTCFrameExt raw_data;
            std::string timezone;
            std::uint16_t year;
            std::uint8_t month;
            std::uint8_t day;
            std::uint8_t hour;
            std::uint8_t min;
            std::uint8_t sec;
            std::uint8_t frame;
            bool reverse;
            
            std::string toString() const {
                return ofVAArgsToString("%04d/%02d/%02d[%s] %02d:%02d:%02d%c%02d",
                                        year, month, day, timezone.c_str(),
                                        hour, min, sec,
                                        (raw_data.ltc.dfbit ? '.' : ':'),
                                        frame);
            }
            
            float receivedTime;
        };
        struct Receiver {
            ~Receiver() {
                ltc_decoder_free(decoder);
                decoder = nullptr;
            }
            
            void setup(const ofSoundStreamSettings &settings,
                       std::size_t channel_offset = 0ul) {
                ofSoundStreamSettings settings_ = settings;
                settings_.setInListener(this);
                this->channel_offset = channel_offset;
                soundStream.setup(settings_);
                
                decoder = ltc_decoder_create(1920, 32);
                total = 0ul;
            }
            
            void onReceive(const std::function<void(Timecode)> &callback)
            { this->callback = callback; };
            
            std::vector<ofSoundDevice> getDeivceList() const
            { return soundStream.getDeviceList(); };
            
            void audioIn(ofSoundBuffer &buffer) {
                std::vector<std::uint8_t> buf;
                getBytePCM(buffer, buf);
                ltc_decoder_write(decoder, buf.data(), buf.size(), total);
                while(ltc_decoder_read(decoder, &frame)) {
                    Timecode timecode;
                    std::memcpy(&timecode.raw_data, &frame, sizeof(frame));
                    SMPTETimecode stime;
                    ltc_frame_to_time(&stime, &frame.ltc, 1);
                    
                    timecode.timezone = stime.timezone;
                    timecode.year = (stime.years < 67)
                                  ? (2000 + stime.years)
                                  : (1900 + stime.years);
                    timecode.month = stime.months;
                    timecode.day = stime.days;
                    timecode.hour = stime.hours;
                    timecode.min = stime.mins;
                    timecode.sec = stime.secs;
                    timecode.frame = stime.frame;
                    timecode.reverse = frame.reverse;
                    timecode.receivedTime = ofGetElapsedTimef();
                    callback(timecode);
                }
                total += buf.size();
            }

        protected:

            void getBytePCM(const ofSoundBuffer &buffer,
                            std::vector<std::uint8_t> &buf) const
            {
                buf.resize(buffer.size() / buffer.getNumChannels());
                std::size_t size = buffer.size();
                for(std::size_t i = 0; i < buf.size(); ++i) {
                    buf[i] = (buffer[buffer.getNumChannels() * i + channel_offset] + 1.0) * 127.5f;
                }
            }
            
            ofSoundStream soundStream;
            LTCDecoder *decoder;
            LTCFrameExt frame;
            std::size_t channel_offset;
            std::size_t total;
            std::function<void(Timecode)> callback{[](Timecode) {}};
        };
    };
};

namespace ofxLTC = ofx::LTC;
using ofxLTCTimecode = ofxLTC::Timecode;
using ofxLTCReceiver = ofxLTC::Receiver;

#endif /* ofxLTC_h */
