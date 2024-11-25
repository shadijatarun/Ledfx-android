🎵 Ledfx android - Audio Reactive LED Effects

Ledfx is an Android application that captures audio from the device's built-in microphone, performs real-time spectral analysis, and visualizes the resulting spectral data to control a connected WLed device. The app translates the audio signal into dynamic, reactive LED effects that change based on the audio frequency and amplitude.


🚀 Features

•	🎧 Audio Reactive LED Effects
Real-time audio processing that transforms sound into dazzling LED effects.

•	📊 Real-Time Spectral Analysis
Utilizes Fast Fourier Transform (FFT) to analyze the audio spectrum.

•	📡 WLED Device Integration
Seamlessly communicate with WLED-compatible devices via UDP.

•	⚙️ Customizable LED Configuration
Adjust IP address, port, and LED count to suit your setup.
________________________________________

🛠️ How It Works
1.	🎙️ Audio Capture
Captures sound from the device's microphone using Android's audio APIs (AAudio or OpenSL ES).
2.	📈 Spectral Analysis
Processes the audio signal using FFT to extract its frequency and amplitude components.
3.	🌈 LED Control
Converts spectral data into dynamic LED effects and sends them to the WLED device.
4.	🖥️ User Interface
Provides an intuitive interface to configure LED parameters and start/stop effects.
________________________________________

📸 Screenshots

 ![image](https://github.com/user-attachments/assets/34eb5e4b-cc92-41c4-9cc3-57fd2078f5ff)

________________________________________

📦 Installation
1.	Clone the repository:
bash
Copy code
git clone https://github.com/shadijatarun/Ledfx-android.git  
2.	Open the project in Android Studio.
3.	Build and install the app on your device.
4.	Configure your WLED device and start enjoying reactive LED effects!
________________________________________

⚙️ Configuration
1.	WLED Setup:
o	Ensure your WLED device is on the same network as your phone.
o	Obtain the device's IP address and port.
2.	App Setup:
o	Open the app.
o	Enter the WLED device IP, port, and number of LEDs in the settings.
3.	Start Visuals:
o	Tap "Start" to begin streaming LED effects.
________________________________________

🤝 Contributing

We welcome contributions! Here's how you can help:

•	Report bugs or suggest features via issues.

•	Fork the repository, make changes, and submit a pull request.
________________________________________
🛡️ License

This project is licensed under the MIT License.
________________________________________
🌟 Support

If you like this project, consider giving it a ⭐ on GitHub!
________________________________________
🧑‍💻 Acknowledgements


•	[WLED Project](https://kno.wled.ge/)

•	[Android's Oboe APIs](https://developer.android.com/games/sdk/oboe)

•	[LedFx Desktop](https://github.com/LedFx/LedFx) 

