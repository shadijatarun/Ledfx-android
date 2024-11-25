//
// Created by Tarun.S on 13-11-2024.
//

#ifndef LEDFX_EXPFILTER_H
#define LEDFX_EXPFILTER_H

#include <cassert>
#include <vector>
#include <algorithm>
#include <numeric>


class ExpFilter {
public:
    ExpFilter(float val = 0.0, float alphaDecay = 0.5, float alphaRise = 0.5,bool init=false, uint32_t size = 24);

    float update(float newVal);

    std::vector<float>& update(float* newVal,uint32_t size);

    float value;
    std::vector<float> valueVec;
private:
    float _alphaDecay,_alphaRise;
    bool _initialized;
};


#endif //LEDFX_EXPFILTER_H
