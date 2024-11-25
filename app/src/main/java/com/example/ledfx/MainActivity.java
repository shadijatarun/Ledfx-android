package com.example.ledfx;

import static com.example.ledfx.AudioCaptureForegroundService.ACTION_START;
import static com.example.ledfx.AudioCaptureForegroundService.ACTION_STOP;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.AdapterView;
import android.widget.Toast;

import com.example.ledfx.databinding.ActivityMainBinding;

public class MainActivity extends Activity
        implements ActivityCompat.OnRequestPermissionsResultCallback {

    private ActivityMainBinding binding;

    // Tag for logging
    private static final String TAG = MainActivity.class.getName();
    // Request code for permission request
    private static final int AUDIO_EFFECT_REQUEST = 0;
    private static final int OBOE_API_AAUDIO = 0;
    private static final int OBOE_API_OPENSL_ES=1;

    private TextView statusText;
    private Button toggleEffectButton;
    private AudioDeviceSpinner recordingDeviceSpinner;
    private boolean isPlaying = false;
    private boolean isRequestPermissionGranted = false;

    private int apiSelection = OBOE_API_AAUDIO;
    private boolean mAAudioRecommended = true;

    /**
     * Initializes the MainActivity and sets up the UI components and initial configurations.
     * It also handles permissions for recording audio and sets the necessary audio stream properties.
     *
     * @param savedInstanceState The saved instance state for activity.
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        TextView status = binding.sampleTextInput;
        status.setText("Audio Reactive Leds");

        Button startButton = binding.buttonStartRecording;

        // Ensures buttons are not null and sets up a click listener
        if (startButton != null ) {
            startButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    toggleEffect();

                    // Update button text depending on current effect state
                    if(isPlaying)
                        startButton.setText("STOP RECORDING");
                    else startButton.setText("START RECORDING");
                }
            });
        }

        // Set up recording device spinner to handle device selection for input
        recordingDeviceSpinner = binding.recordingDevicesSpinner;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            recordingDeviceSpinner.setDirectionType(AudioManager.GET_DEVICES_INPUTS);
            recordingDeviceSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                    LedfxEngine.setRecordingDeviceId(getRecordingDeviceId());
                }

                @Override
                public void onNothingSelected(AdapterView<?> adapterView) {
                    // Do nothing
                }
            });
        }

        // Request permission if not granted
        if (!isRecordPermissionGranted()){
            requestRecordPermission();
        } else {
            isRequestPermissionGranted = true;
        }

        // Initialize the default audio stream values and configure volume control stream
        LedfxEngine.setDefaultStreamValues(this);
        setVolumeControlStream(AudioManager.STREAM_MUSIC);

        onStartTest();
    }

    /**
     * Enables or disables the Audio API UI elements based on the given flag.
     * This is to ensure the proper selection of API during the setup.
     *
     * @param enable Flag to enable/disable UI elements.
     */
    private void EnableAudioApiUI(boolean enable) {
        if(apiSelection == OBOE_API_AAUDIO && !mAAudioRecommended) {
            apiSelection = OBOE_API_OPENSL_ES;
        }
        setSpinnersEnabled(enable);
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    /**
     * Called when the activity is destroyed. Stops any ongoing test and shuts down services.
     */
    @Override
    protected void onDestroy() {
        onStopTest();

        // Stop the audio capture foreground service if running
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Intent serviceIntent = new Intent(ACTION_STOP, null, this,
                    AudioCaptureForegroundService.class);
            startForegroundService(serviceIntent);
        }
        super.onDestroy();
    }

    /**
     * Starts the necessary audio setup and configures the system to use the correct API.
     */
    private void onStartTest() {
        LedfxEngine.create();
        mAAudioRecommended = LedfxEngine.isAAudioRecommended();
        EnableAudioApiUI(true);
        LedfxEngine.setAPI(apiSelection);
    }

    /**
     * Stops the effect and cleans up resources used by the engine.
     */
    private void onStopTest() {
        stopEffect();
        LedfxEngine.delete();
    }

    /**
     * Toggles between starting and stopping the LED effect based on the current state.
     */
    public void toggleEffect() {
        if (isPlaying) {
            stopEffect();
        } else {
            LedfxEngine.setAPI(apiSelection);
            startEffect();
        }
    }

    /**
     * Starts the LED effect by configuring the necessary parameters like IP, port, and number of LEDs.
     * Also initiates the service to capture audio data.
     */
    private void startEffect() {
        Log.d(TAG, "Attempting to start");

        // Check if permission is granted for recording audio
        if (!isRequestPermissionGranted){
            return;
        }

        // Get configuration data from UI input
        String ip = binding.ipAddressInput.getText().toString();
        int port = Integer.parseInt(binding.portNumberInput.getText().toString());
        long numLeds = Long.parseLong(binding.numLedsInput.getText().toString());

        // Update LED configuration and start effect
        LedfxEngine.updateConfig(ip, port, numLeds);
        boolean success = LedfxEngine.setEffectOn(true);

        if (success) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                Intent serviceIntent = new Intent(ACTION_START, null, this,
                        AudioCaptureForegroundService.class);
                startForegroundService(serviceIntent);
            }
            isPlaying = true;
            EnableAudioApiUI(false);
        } else {
            isPlaying = false;
        }
    }

    /**
     * Stops the LED effect and resets the status UI.
     */
    private void stopEffect() {
        Log.d(TAG, "Playing, attempting to stop");
        LedfxEngine.setEffectOn(false);
        resetStatusView();
        isPlaying = false;
        EnableAudioApiUI(true);
    }

    /**
     * Enables or disables the spinner UI elements based on the provided flag.
     *
     * @param isEnabled Flag to enable/disable the spinners.
     */
    private void setSpinnersEnabled(boolean isEnabled){
        recordingDeviceSpinner.setEnabled(isEnabled);
    }

    /**
     * Gets the ID of the currently selected recording device from the spinner.
     *
     * @return The selected recording device's ID.
     */
    private int getRecordingDeviceId(){
        return ((AudioDeviceListEntry)recordingDeviceSpinner.getSelectedItem()).getId();
    }

    /**
     * Checks if the app has the necessary permission to record audio.
     *
     * @return True if the permission is granted, false otherwise.
     */
    private boolean isRecordPermissionGranted() {
        return (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED);
    }

    /**
     * Requests the user for permission to record audio.
     */
    private void requestRecordPermission(){
        ActivityCompat.requestPermissions(
                this,
                new String[]{Manifest.permission.RECORD_AUDIO},
                AUDIO_EFFECT_REQUEST);
    }

    /**
     * Resets the status view to a default state.
     */
    private void resetStatusView() {
    }

    /**
     * Handles the result of the permission request.
     * If permission is granted, the effect is enabled.
     *
     * @param requestCode The request code for the permission request.
     * @param permissions The permissions requested.
     * @param grantResults The results of the permission request.
     */
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {

        if (AUDIO_EFFECT_REQUEST != requestCode) {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
            return;
        }

        if (grantResults.length != 1 ||
                grantResults[0] != PackageManager.PERMISSION_GRANTED) {

            // User denied the permission
            Toast.makeText(getApplicationContext(),
                            "Permission denied",
                            Toast.LENGTH_SHORT)
                    .show();
        } else {
            // Permission granted, start live effect
            isRequestPermissionGranted = true;
        }
    }
}
