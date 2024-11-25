//
// Created by Tarun.S on 13-11-2024.
//

#include "logging_macros.h"
#include "AubioDspProcessor.h"

/**
 * Constructor for the AubioDspProcessor class.
 * Initializes the necessary components for performing Mel-Bank filtering, including
 * the digital filter, phase vocoder, FFT, and Mel filter bank.
 *
 * @param winS The window size for the phase vocoder and FFT.
 * @param hopS The hop size for the phase vocoder.
 * @param filterS The number of filters in the Mel filter bank.
 * @param sampleRate The sample rate of the audio.
 * @param fMin The minimum frequency for the Mel filter bank.
 * @param fMax The maximum frequency for the Mel filter bank.
 */
AubioDspProcessor::AubioDspProcessor(const size_t winS, const size_t hopS, const size_t filterS,
                                     const float sampleRate, const float fMin, const float fMax) {

    // Initialize digital filter, pre-emphasis phase, set coefficients for biquadratic filter.
    _sample = new_fvec(1);
    _digitalFilter =     new_aubio_filter(3);
    // Change filter response by setting biquad coefficients.
    aubio_filter_set_biquad(_digitalFilter,
                            1.00000285
            ,-1.93078064
            ,0.95054174
            ,-1.93078064
            ,0.95054459
    );
    aubio_filter_set_samplerate(_digitalFilter,sampleRate);

    LOGI("AubioDspProcessor initialized with window size: %zu, hop size: %zu, filter size: %zu, sample rate: %.2f, frequency range: [%.2f, %.2f]",
         winS, hopS, filterS, sampleRate, fMin, fMax);


    // Setup phase vocoder, fft size, window type.
    _phaseVocoder=  new_aubio_pvoc(winS,hopS);
    // Select window type according to your preference.
    aubio_pvoc_set_window(_phaseVocoder,"hanning");
    _fft = new_cvec(winS);

    // Setup melbank, convert frequencies to mel range.
    _melOutput = new_fvec(filterS);
    _filterBank = new_aubio_filterbank(filterS,winS);
    aubio_filterbank_set_norm(_filterBank,1.0f);
    // This seems to works great, without needing to rounding off values.
    aubio_filterbank_set_power(_filterBank,4.0f);
    aubio_filterbank_set_mel_coeffs(_filterBank,sampleRate, fMin,fMax);
}

/**
 * Applies the Mel-Bank filtering process on the provided audio data.
 * This includes applying a digital filter, performing phase vocoder time-frequency analysis,
 * and computing the Mel spectrogram using a filter bank.
 * The results are then passed to the given ExpFilter object for further processing.
 *
 * @param audioData Pointer to the audio data buffer.
 * @param dataS The size of the audio data buffer (in samples).
 * @param melBank A shared pointer to an ExpFilter object that will be updated with Mel output data.
 */
void AubioDspProcessor::doMelBank(void *audioData, const size_t dataS, std::shared_ptr<ExpFilter> melBank) {

    // Set the length of the sample vector to match the audio data size
    _sample->length = dataS;
    _sample->data = (float*)audioData;  // Assign audio data to sample vector

    // Apply the digital filter on the audio sample
    aubio_filter_do(_digitalFilter, _sample);

    // Clear the FFT vector before processing
    cvec_zeros(_fft);
    // Perform phase vocoder processing (time-frequency analysis)
    aubio_pvoc_do(_phaseVocoder, _sample, _fft);

    // Clear the Mel output vector before computation
    fvec_zeros(_melOutput);
    // Apply the Mel filter bank to the FFT result
    aubio_filterbank_do(_filterBank, _fft, _melOutput);

    // Update the ExpFilter with the computed Mel output
    melBank->update(_melOutput->data, _melOutput->length);

}

/**
 * Destructor for the AubioDspProcessor class.
 * Cleans up and releases all dynamically allocated memory for DSP processing components.
 */
AubioDspProcessor::~AubioDspProcessor() {
    // Release the dynamically allocated memory for each DSP component
    del_fvec(_melOutput);
    del_aubio_filterbank(_filterBank);
    del_cvec(_fft);
    del_aubio_pvoc(_phaseVocoder);
//    del_fvec(_sample); // do not delete audio sample data.
    del_aubio_filter(_digitalFilter);
}