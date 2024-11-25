//
// Created by Tarun.S on 20-11-2024.
//

#include <logging_macros.h>

#include "WLedDevice.h"
#include "cassert"

/**
 * @brief Constructor to initialize the WLedDevice class, setting up the socket address structure (_addr).
 * Initializes the address family (IPv4) for the device.
 * This method is called when an object of WLedDevice is created.
 */
WLedDevice::WLedDevice(){
    _addr.sin_family = AF_INET;
}

/**
 * @brief Updates the configuration for the WLedDevice with the specified IP address, port number, and LED count.
 * This function configures the device for communication by setting up its IP, port, and the number of LEDs it will control.
 *
 * @param iPaddr The IP address of the WLed device, formatted as a string (IPv4).
 * @param portNum The port number on which the device listens.
 * @param numLeds The number of LEDs that this device will control (should be between 1 and 490).
 *
 * @throws std::assertion Throws an assertion error if:
 *          - The number of LEDs is not within the range (1 to 490).
 *          - The IP address exceeds the maximum allowed length (15 characters).
 */
void WLedDevice::updateConfig(std::string iPaddr, uint16_t portNum, size_t numLeds) {
    // Ensure the number of LEDs is between 1 and 490 (valid range)
    assert((((1 > numLeds) && (490 > numLeds)) == true ) && "Invalid LED count. LED count must be between 1 and 490.");
    // Ensure the IP address is valid (max length of 15 characters for IPv4)
    assert(16 > iPaddr.size() && "Invalid IP Address. The address exceeds max length of 15 characters.");

    _addr.sin_port = htons(portNum); // Port number
    _addr.sin_addr.s_addr = inet_addr(iPaddr.c_str()); // IP address

    _numLeds = numLeds;
}

/**
 * @brief Activates the WLedDevice by opening a UDP socket and connecting to the device at the specified IP address and port.
 * This function attempts to create a socket for communication and connect to the device. If successful, it returns true.
 *
 * @return true if the socket is successfully created and connected to the device, false otherwise.
 */
bool WLedDevice::activate() {
bool res(true);

    _sckt = socket(AF_INET,SOCK_DGRAM,0);

    if (!_sckt) {
        res = false;
        LOGE("Not able to open socket for wled device");
    } else{
        if(0> connect(_sckt,(sockaddr*)&_addr,sizeof(_addr))){
            LOGE("Failed to connect to device, with the provided address & port");
        }
    }
    return res;
}

/**
 * @brief Deactivates the WLedDevice by closing the open socket.
 * This function cleans up by closing the previously opened socket and releasing resources associated with it.
 *
 * @return true if the socket is successfully closed, false if there was an error.
 */
bool WLedDevice::deactivate() {
    bool res(false);

    if(_sckt){
        if(0 > close(_sckt))
            LOGE("Failed to closed already opened socket for wled device");
        else
            res= true;
    }
    return res;
}

/**
 * @brief Sends LED control data to the WLedDevice over the active socket.
 * The data includes control bytes (protocol and timeout) and LED data.
 * This method sends the data over the network to control the LEDs based on the configured settings.
 *
 * @param data Pointer to the data buffer containing LED control data and protocol information.
 * @param numBytes The total number of bytes to be sent, which should match the expected data length.
 *
 * @return true if the data was successfully sent to the device, false otherwise.
 */
bool WLedDevice::flush(uint8_t *data, size_t numBytes) {
    bool res(false);
    // check total bytes is equal to LEDS * bytes for each led + protocol selection byte + timeout selection byte.
    if(((_numLeds*_byteCountForEachLed+2) == numBytes) && (_sckt)){
        data[0]=_protocol;
        data[1]=_timeOutSec;
        if(!send(_sckt,data,numBytes,0))
            LOGE("Failed to send data, socket error");
        else
            res= true;

    } else{
        LOGE("Failed to send data, check number of leds or socket descriptor");
    }
    return res;
}