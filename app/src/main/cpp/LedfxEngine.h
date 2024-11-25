/*
 * Copyright 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LEDFX_LEDFXENGINE_H
#define LEDFX_LEDFXENGINE_H

#include <jni.h>
#include <oboe/Oboe.h>
#include <string>
#include <thread>
#include <array>
#include "AubioDspProcessor.h"
#include "IDspProcessor.h"
#include "WLedDevice.h"

#define SAMPLE_RATE 44100u
#define FFT_SIZE 512u
#define HOP_SIZE 192U  // same as frame size.
#define FILTER_SIZE 24u
#define MIN_FREQ_HZ 200u
#define MAX_FREQ_HZ 4000u
#define BYTES_PER_LED 3u

class LedfxEngine : public oboe::AudioStreamCallback {
public:
    LedfxEngine();

    void setRecordingDeviceId(int32_t deviceId);

    /**
     * @param isOn
     * @return true if it succeeds
     */
    bool setEffectOn(bool isOn);

    /*
     * oboe::AudioStreamDataCallback interface implementation
     */
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream,
                                          void *audioData, int32_t numFrames) override;

    /*
     * oboe::AudioStreamErrorCallback interface implementation
     */
    void onErrorBeforeClose(oboe::AudioStream *oboeStream, oboe::Result error) override;
    void onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) override;

    bool setAudioApi(oboe::AudioApi);
    bool isAAudioRecommended(void);
    void updateConfig(std::string iPaddr, uint16_t portNum, size_t numLeds);

private:
    bool              _isEffectOn = false;
    int32_t           _recordingDeviceId = oboe::kUnspecified;
    const oboe::AudioFormat _format = oboe::AudioFormat::Float; // for easier processing
    oboe::AudioApi    _audioApi = oboe::AudioApi::AAudio;
    int32_t           _sampleRate = SAMPLE_RATE;
    const int32_t     _inputChannelCount = oboe::ChannelCount::Stereo;

    std::unique_ptr<IDspProcessor> _dspProcessor;

    std::shared_ptr<oboe::AudioStream> _recordingStream;

    std::shared_ptr<ExpFilter> _inVolFilter;
    std::shared_ptr<ExpFilter> _melBankOutput;
    std::shared_ptr<std::vector<uint8_t>> _ledData;
    std::shared_ptr<WLedDevice> _device;


    oboe::Result openStreams();

    void closeStreams();

    void closeStream(std::shared_ptr<oboe::AudioStream> &stream);

    oboe::AudioStreamBuilder *setupCommonStreamParameters(
        oboe::AudioStreamBuilder *builder);
    oboe::AudioStreamBuilder *setupRecordingStreamParameters(
        oboe::AudioStreamBuilder *builder, int32_t sampleRate);
        void warnIfNotLowLatency(std::shared_ptr<oboe::AudioStream> &stream);
};

#endif  // LEDFX_LEDFXENGINE_H
