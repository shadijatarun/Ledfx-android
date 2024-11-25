//
// Created by Tarun.S on 13-11-2024.
//

#ifndef LEDFX_AUBIODSPPROCESSOR_H
#define LEDFX_AUBIODSPPROCESSOR_H

#include "ExpFilter.h"
#include "types.h"
#include "cvec.h"
#include "fvec.h"
#include "spectral/phasevoc.h"
#include "oboe/Oboe.h"
#include "aubio.h"
#include "IDspProcessor.h"

class AubioDspProcessor : public IDspProcessor {
    AubioDspProcessor()= delete;

fvec_t* _sample = nullptr;
aubio_filter_t* _digitalFilter = nullptr;
aubio_pvoc_t* _phaseVocoder = nullptr;
aubio_filterbank_t* _filterBank = nullptr;
fvec_t* _melOutput = nullptr;
cvec_t* _fft = nullptr;

public:

    AubioDspProcessor(const size_t winS, const size_t hopS, const size_t filterS, const float sampleRate, const float fMin, const float fMax);

    void doMelBank(void* audioData, const size_t dataS, std::shared_ptr<ExpFilter> melBank);
    ~AubioDspProcessor();
};


#endif //LEDFX_AUBIODSPPROCESSOR_H
