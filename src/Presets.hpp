#pragma once

#include "LeniaEngine.hpp"
#include <vector>
#include <string>

namespace lenia {

struct MultiChannelPresetRule {
    float ringWeights[16];
    float mu;
    float sigma;
    float growthStrength;
    float radiusFraction;
    int   numRings;
    int   sourceChannel;
    int   destChannel;
    int   kernelType;
    int   growthType;
};

struct MultiChannelPreset {
    const char*  name;
    const char*  category;
    int          radius;
    float        dt;
    int          gridW;
    int          gridH;
    int          numChannels;
    int          numRules;
    MultiChannelPresetRule rules[16];
    int          cellRows;
    int          cellCols;
    const float* cellsCh0;
    const float* cellsCh1;
    const float* cellsCh2;
};

const std::vector<Preset>& getPresets();
const std::vector<std::string>& getPresetCategories();
const std::vector<KernelPreset>& getKernelPresets();
const std::vector<MultiChannelPreset>& getMultiChannelPresets();

}
