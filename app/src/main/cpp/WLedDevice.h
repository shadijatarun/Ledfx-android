//
// Created by Tarun.S on 20-11-2024.
//

#ifndef LEDFX_WLEDDEVICE_H
#define LEDFX_WLEDDEVICE_H

#include <unistd.h>
#include <types.h>
#include <numeric>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


class WLedDevice {
size_t _numLeds =0u;
uint8_t _timeOutSec = 1u;
uint8_t _protocol = 2u; // DRGB protocol.
uint8_t _byteCountForEachLed = 3u;
int _sckt = -1;
sockaddr_in _addr;

public:
WLedDevice();
bool activate(void);
bool deactivate(void);
bool flush(uint8_t* data, size_t numBytes);
void updateConfig(std::string ipAddr, uint16_t portNum, size_t numLeds);
};


#endif //LEDFX_WLEDDEVICE_H
