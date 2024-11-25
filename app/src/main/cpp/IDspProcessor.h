//
// Created by Tarun.S on 18-11-2024.
//

#ifndef LEDFX_IDSPPROCESSOR_H
#define LEDFX_IDSPPROCESSOR_H

#include <memory>
#include "ExpFilter.h"

/**
 * @brief class for DSP (Digital Signal Processing) processing.
 * This class defines the interface for processing audio data with a mel frequency bank filter.
 */
class IDspProcessor {
public:
    /**
     * Pure virtual function that performs Mel-Bank processing on the provided audio data.
     *
     * @param audioData A pointer to the audio data buffer that will be processed.
     * @param dataS The size of the audio data buffer in bytes.
     * @param melBank A shared pointer to an ExpFilter object, which is used to process the Mel-Bank.
     */
    virtual void doMelBank(void* audioData, const size_t dataS, std::shared_ptr<ExpFilter> melBank) = 0;

    /**
     * Virtual destructor for the interface, ensuring proper cleanup of derived classes.
     */
    virtual ~IDspProcessor() {}
};



#endif //LEDFX_IDSPPROCESSOR_H
