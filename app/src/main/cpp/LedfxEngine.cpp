#include <cassert>
#include <logging_macros.h>

#include "LedfxEngine.h"
#include "ExpFilter.h"

#include "types.h"
#include "cvec.h"
#include "fvec.h"
#include "spectral/phasevoc.h"
#include "oboe/Oboe.h"
#include "aubio.h"
#include <cassert>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/**
 * @brief Constructor for the LedfxEngine class.
 * This constructor initializes all necessary components for the LED effects engine,
 * including filters, digital signal processing (DSP) processor, and LED device control.
 * It sets up the mel filter bank, input volume filter, DSP processor, and prepares the data structure for controlling LEDs.
 */
LedfxEngine::LedfxEngine() {

    // Initialize the mel filter bank with an initial value of 0.0, using a decay factor of 0.70 and a rise factor of 0.90.
    // This is used for processing frequency data with a smooth transition.
    // The filter is initially not enabled (false), and the filter size is defined by FILTER_SIZE.
    _melBankOutput = std::make_shared<ExpFilter>(0.0f, 0.70f, 0.90f, false, FILTER_SIZE);
    LOGD("Mel filter bank initialized with decay (0.70) and rise (0.90) factors.");

    // Initialize the input volume filter with an initial value of -90.0 dB, and both decay and rise factors set to 0.99.
    // This filter will smooth the volume input over time to avoid abrupt changes.
    // It is initially enabled (true), and the size is set to 1 (representing a single value).
    _inVolFilter = std::make_shared<ExpFilter>(-90.0f, 0.99f, 0.99f, true, 1u);
    LOGD("Input volume filter initialized with high smoothing factors (0.99) for decay and rise.");

    // Initialize the DSP processor with the given FFT size, hop size, filter size, sample rate, and frequency range.
    // The DSP processor will process incoming audio data and extract features like frequency bins.
    _dspProcessor = std::make_unique<AubioDspProcessor>(FFT_SIZE, HOP_SIZE, FILTER_SIZE, SAMPLE_RATE, MIN_FREQ_HZ, MAX_FREQ_HZ);
    LOGD("DSP Processor (Aubio) initialized with FFT_SIZE = %i, HOP_SIZE = %i, and sample rate = %i Hz.", FFT_SIZE, HOP_SIZE, SAMPLE_RATE);

    // Initialize the LED device controller, which will handle communication with the physical LED hardware.
    _device = std::make_shared<WLedDevice>();
    LOGD("LED Device controller initialized.");

    // Prepare a shared vector to hold the LED data. This data structure will contain the LED control data
    // (color, brightness, etc.) for all 60 LEDs, with each LED's data occupying `BYTES_PER_LED` bytes.
    // An additional 2 bytes are reserved for protocol and timeout information.
    _ledData = std::make_shared<std::vector<uint8_t>>((60 * BYTES_PER_LED) + 2, 0u);
    LOGD("LED data structure initialized for 60 LEDs with %d bytes per LED.", BYTES_PER_LED);

    // At this point, all core components are initialized and ready for use.
}


/**
 * Sets the ID of the device used for recording audio.
 * @param deviceId The ID of the recording device to set.
 */
void LedfxEngine::setRecordingDeviceId(int32_t deviceId) {
    _recordingDeviceId = deviceId;
}

/**
 * Checks if AAudio is the recommended API for audio streaming.
 * @return True if AAudio is recommended, otherwise false.
 */
bool LedfxEngine::isAAudioRecommended() {
    return oboe::AudioStreamBuilder::isAAudioRecommended();
}

/**
 * Sets the audio API for the application. This method will fail if
 * the effect is currently enabled.
 * @param api The audio API to set.
 * @return True if the audio API was successfully set, otherwise false.
 */
bool LedfxEngine::setAudioApi(oboe::AudioApi api) {
    if (_isEffectOn) return false;
    _audioApi = api;
    return true;
}

/**
 * Turns the effect on or off. If turning the effect on, it opens audio streams.
 * If turning the effect off, it closes the streams.
 * @param isOn If true, the effect is turned on; if false, the effect is turned off.
 * @return True if the effect was successfully set, otherwise false.
 */
bool LedfxEngine::setEffectOn(bool isOn){
    bool success = true;
    if (isOn != _isEffectOn) {
        if (isOn) {
            success = openStreams() == oboe::Result::OK;
            if (success) {
                _isEffectOn = isOn;
            }
        } else {
            closeStreams();
            _isEffectOn = isOn;
        }
    }
    return success;
}

/**
 * Updates the configuration for the LED device, including IP address, port number, and the number of LEDs.
 * Resizes the LED data buffer accordingly.
 * @param iPaddr The IP address of the LED device.
 * @param portNum The port number of the LED device.
 * @param numLeds The number of LEDs to configure.
 */
void LedfxEngine::updateConfig(std::string iPaddr, uint16_t portNum, size_t numLeds) {
    if(_device){
        _device->updateConfig(iPaddr,portNum,numLeds);
        _ledData->resize((numLeds*BYTES_PER_LED)+2,0);
    }
}

/**
 * Closes the audio streams in the correct order, ensuring the playback stream is stopped before the recording stream.
 */
void LedfxEngine::closeStreams() {
    /*
    * Note: The order of events is important here.
    * The playback stream must be closed before the recording stream. If the
    * recording stream were to be closed first the playback stream's
    * callback may attempt to read from the recording stream
    * which would cause the app to crash since the recording stream would be
    * null.
    */
    closeStream(_recordingStream);
    _device->deactivate();
}

/**
 * Opens the audio streams, starting with the playback stream and using its properties to configure the recording stream.
 * @return The result of the stream opening operation (OK if successful, otherwise an error result).
 */
oboe::Result  LedfxEngine::openStreams() {
    // Note: The order of stream creation is important. We create the playback
    // stream first, then use properties from the playback stream
    // (e.g. sample rate) to create the recording stream. By matching the
    // properties we should get the lowest latency path

    if(!_device->activate())
        LOGE("Failed to activate device");

    // Create and setup builder for input stream.
    oboe::AudioStreamBuilder inBuilder;

    setupRecordingStreamParameters(&inBuilder, _sampleRate);
    auto result = inBuilder.openStream(_recordingStream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to open input stream. Error %s", oboe::convertToText(result));
        return result;
    }
    warnIfNotLowLatency(_recordingStream);

    result = _recordingStream->requestStart();

    return result;
}

/**
 * Sets the stream parameters for the recording stream, including sample rate and channel count.
 * @param builder The recording stream builder to configure.
 * @param sampleRate The desired sample rate of the recording stream.
 * @return The builder with the updated parameters.
 */
oboe::AudioStreamBuilder *LedfxEngine::setupRecordingStreamParameters(
        oboe::AudioStreamBuilder *builder, int32_t sampleRate) {
    // This sample uses blocking read() because we don't specify a callback
    builder->setDeviceId(_recordingDeviceId)
            ->setDataCallback(this)
            ->setErrorCallback(this)
            ->setDirection(oboe::Direction::Input)
            ->setSampleRate(sampleRate)
            ->setChannelCount(_inputChannelCount);
    return setupCommonStreamParameters(builder);
}


/**
 * Sets the stream parameters common to both recording and playback streams.
 * @param builder The builder to configure.
 * @return The builder with common stream parameters.
 */
oboe::AudioStreamBuilder *LedfxEngine::setupCommonStreamParameters(
        oboe::AudioStreamBuilder *builder) {
    // We request EXCLUSIVE mode since this will give us the lowest possible
    // latency.
    // If EXCLUSIVE mode isn't available the builder will fall back to SHARED
    // mode.
    builder->setAudioApi(_audioApi)
            ->setFormat(_format)
            ->setFormatConversionAllowed(true)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency);
    return builder;
}

/**
 * Closes the specified stream. Stops the stream and releases resources.
 * @param stream The stream to close.
 */
void LedfxEngine::closeStream(std::shared_ptr<oboe::AudioStream> &stream) {
    if (stream) {
        oboe::Result result = stream->stop();
        if (result != oboe::Result::OK) {
            LOGW("Error stopping stream: %s", oboe::convertToText(result));
        }
        result = stream->close();
        if (result != oboe::Result::OK) {
            LOGE("Error closing stream: %s", oboe::convertToText(result));
        } else {
            LOGW("Successfully closed streams");
        }
        stream.reset();
    }
}

/**
 * Warn in logcat if non-low latency stream is created
 * @param stream: newly created stream
 */
void LedfxEngine::warnIfNotLowLatency(std::shared_ptr<oboe::AudioStream> &stream) {
    if (stream->getPerformanceMode() != oboe::PerformanceMode::LowLatency) {
        LOGW(
                "Stream is NOT low latency."
                "Check your requested format, sample rate and channel count");
    }
}

/**
 * Handles the audio data ready event for playback. This method reads from the input stream and processes
 * the audio data for LED visualization.
 * @param oboeStream The playback stream requesting additional samples.
 * @param audioData The buffer to load audio samples for the playback stream.
 * @param numFrames The number of frames to load into the audioData buffer.
 * @return DataCallbackResult::Continue to keep processing audio data.
 */
oboe::DataCallbackResult LedfxEngine::onAudioReady(
        oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {


    auto samp = new_fvec(1u);
    samp->length = numFrames*2;
    samp->data=(float*)audioData;
    auto vol = 1+ aubio_db_spl(samp)/100;
    vol=std::max<float>(0,std::min<float>(1,vol));
    _inVolFilter->update(vol);

    if(_inVolFilter->value >= 0.7 ){ // default value 0.9

        _dspProcessor->doMelBank(audioData, numFrames * 2, _melBankOutput);

        auto calMelAvrg = [](std::vector<float>::iterator start, uint32_t size,float min)->uint8_t {
           float val =  (std::accumulate(start,start+(size-1),0.0f))/size;
//            return val >= min ? 1u:0u;
            return (uint8_t)val;
        };

        uint8_t r,g,b;

        r=calMelAvrg(_melBankOutput->valueVec.begin(), 4u, 1.0f);
        g=calMelAvrg(_melBankOutput->valueVec.begin() + 4, 4u, 1.0f);
        b=calMelAvrg(_melBankOutput->valueVec.begin() + 6, 4u, 1.0f);

        for(uint8_t i= 2; i<=180;i+=3){

          (*_ledData)[i]=static_cast<uint8_t>(r);
            (*_ledData)[i+1]=static_cast<uint8_t>(g);
            (*_ledData)[i+2]=static_cast<uint8_t>(b);
        }

    } else{
         std::fill(_ledData->begin(),_ledData->end(),0u);
    }
    _device->flush(_ledData->data(),_ledData->size());
//        del_fvec(samp);
    return oboe::DataCallbackResult::Continue;
}

/**
 * Handles errors before closing the stream, typically logging errors before
 * any stream shutdown operations.
 * @param oboeStream The stream to close.
 * @param error The error causing the stream to close.
 */
void LedfxEngine::onErrorBeforeClose(oboe::AudioStream *oboeStream,
                                     oboe::Result error) {
    LOGE("%s stream Error before close: %s",
         oboe::convertToText(oboeStream->getDirection()),
         oboe::convertToText(error));
}

/**
 * Handles errors after the stream is closed.
 * @param oboeStream The stream that has been closed.
 * @param error The error that caused the stream to close.
 */
void LedfxEngine::onErrorAfterClose(oboe::AudioStream *oboeStream,
                                    oboe::Result error) {
    LOGE("%s stream Error after close: %s",
         oboe::convertToText(oboeStream->getDirection()),
         oboe::convertToText(error));

    closeStreams();

    // Restart the stream if the error is a disconnect.
    if (error == oboe::Result::ErrorDisconnected) {
        LOGI("Restarting LedfxEngine");
        openStreams();
    }
}
