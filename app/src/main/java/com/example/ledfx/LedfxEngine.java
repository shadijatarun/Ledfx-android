package com.example.ledfx;

import android.content.Context;
import android.media.AudioManager;
import android.os.Build;

/**
 * Singleton class for interacting with the LED effects engine.
 * This class interfaces with the native "ledfx-native" library to control LED effects based on audio input.
 */
public enum LedfxEngine {

    INSTANCE;

    // Load the native library for LED effects
    static {
        System.loadLibrary("ledfx-native");
    }

    // Native methods for interacting with the LED effects engine
    /**
     * Initializes the LED effects engine.
     *
     * @return true if initialization was successful, false otherwise.
     */
    static native boolean create();

    /**
     * Checks if the AAudio API is recommended for audio processing.
     *
     * @return true if AAudio is recommended, false otherwise.
     */
    static native boolean isAAudioRecommended();

    /**
     * Sets the API type for audio processing.
     *
     * @param apiType The type of audio API to use (e.g., AAudio, OpenSL ES).
     * @return true if the API was set successfully, false otherwise.
     */
    static native boolean setAPI(int apiType);

    /**
     * Turns the LED effect on or off.
     *
     * @param isEffectOn true to start the effect, false to stop it.
     * @return true if the effect was successfully turned on/off, false otherwise.
     */
    static native boolean setEffectOn(boolean isEffectOn);

    /**
     * Updates the LED configuration with the given parameters.
     *
     * @param iPAddr The IP address of the LED device.
     * @param portNum The port number to connect to.
     * @param numLeds The number of LEDs in the device.
     */
    static native void updateConfig(String iPAddr, int portNum, long numLeds);

    /**
     * Sets the recording device ID for audio input.
     *
     * @param deviceId The ID of the audio recording device.
     */
    static native void setRecordingDeviceId(int deviceId);

    /**
     * Cleans up and deletes the LED effects engine resources.
     */
    static native void delete();

    /**
     * Sets default stream values for audio input/output, such as sample rate and frames per burst.
     *
     * @param defaultSampleRate The default sample rate for audio streams.
     * @param defaultFramesPerBurst The default number of frames per burst for audio processing.
     */
    static native void native_setDefaultStreamValues(int defaultSampleRate, int defaultFramesPerBurst);

    /**
     * Initializes the default stream values based on the system's audio manager settings.
     *
     * @param context The context used to access system services.
     */
    static void setDefaultStreamValues(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            AudioManager myAudioMgr = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

            // Retrieve default sample rate and frames per burst from system properties
            String sampleRateStr = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            int defaultSampleRate = Integer.parseInt(sampleRateStr);
            String framesPerBurstStr = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
            int defaultFramesPerBurst = Integer.parseInt(framesPerBurstStr);

            // Pass the retrieved values to the native method
            native_setDefaultStreamValues(defaultSampleRate, defaultFramesPerBurst);
        }
    }
}
