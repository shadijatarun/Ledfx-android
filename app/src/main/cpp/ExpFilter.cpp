//
// Created by Tarun.S on 13-11-2024.
//

#include "ExpFilter.h"

/**
 * Constructor for the ExpFilter class.
 * Initializes the filter with specified values for decay and rise smoothing factors,
 * as well as setting the initial value and size of the valueVec.
 *
 * @param val Initial value of the filter.
 * @param alphaDecay Decay smoothing factor, between 0.0 and 1.0.
 * @param alphaRise Rise smoothing factor, between 0.0 and 1.0.
 * @param init Boolean indicating whether the filter is initialized or not.
 * @param size The size of the valueVec.
 */
ExpFilter::ExpFilter(float val, float alphaDecay, float alphaRise, bool init, uint32_t size):
_alphaDecay(alphaDecay), _alphaRise(alphaRise), value(val), _initialized(init), valueVec(size) {
    assert(0.0 < _alphaDecay && _alphaDecay < 1.0 && "Invalid decay smoothing factor");
    assert(0.0 < _alphaRise && _alphaRise < 1.0 && "Invalid rise smoothing factor");
}

/**
 * Updates the filter's value with a new input value, using exponential smoothing
 * based on whether the new value is greater than or less than the current value.
 *
 * @param newVal The new value to be used for filtering.
 * @return The updated filtered value.
 */
float ExpFilter::update(float newVal) {
    if (!_initialized) {
        value = newVal;
        _initialized = true;
        return value;
    }

    float alpha = newVal > value ? _alphaRise : _alphaDecay;
    value = alpha * newVal + (1.0f - alpha) * value;
    return value;
}

/**
 * Updates the filter's valueVec with an array of new values, each value is updated
 * using its corresponding smoothing factor based on whether the new value is greater
 * than or less than the previous value.
 *
 * @param newVal Pointer to the array of new values to be filtered.
 * @param size The number of elements in the newVal array.
 * @return The updated valueVec after filtering.
 */
std::vector<float>&  ExpFilter::update(float* newVal,uint32_t size){

    for(size_t i =0; i <size; i++){
        float diff = (newVal[i] - valueVec[i]) > 0.0f ? _alphaRise : _alphaDecay;
        valueVec[i]= diff * newVal[i] + (1.0f - diff) * valueVec[i];
    }
    return valueVec;
}
