/**
 * @file Presets.hpp
 * @brief Preset definitions for known Lenia species and configurations.
 * 
 * Contains parameter sets for reproducing discovered Lenia creatures
 * and interesting patterns. Multi-channel presets support interactions
 * between multiple cell types.
 */

#pragma once

#include "LeniaEngine.hpp"
#include <vector>
#include <string>

namespace lenia {

/**
 * @brief Rule definition for multi-channel Lenia.
 * 
 * In multi-channel mode, each rule specifies how one channel
 * affects another through its own kernel and growth function.
 */
struct MultiChannelPresetRule {
    float ringWeights[16];     // Kernel ring weights
    float mu;                  // Growth function center
    float sigma;               // Growth function width
    float growthStrength;      // Scaling factor for growth
    float radiusFraction;      // Fraction of base radius
    int   numRings;            // Number of kernel rings
    int   sourceChannel;       // Channel to read from
    int   destChannel;         // Channel to write to
    int   kernelType;          // Kernel shape
    int   growthType;          // Growth function type
};

/**
 * @brief Multi-channel Lenia preset with cross-channel interactions.
 */
struct MultiChannelPreset {
    const char*  name;         // Display name
    const char*  category;     // Category for UI grouping
    int          radius;       // Base kernel radius
    float        dt;           // Time step
    int          gridW;        // Grid width
    int          gridH;        // Grid height
    int          numChannels;  // Number of channels (1-3)
    int          numRules;     // Number of interaction rules
    MultiChannelPresetRule rules[16];  // Up to 16 rules
    int          cellRows;     // Initial pattern height
    int          cellCols;     // Initial pattern width
    const float* cellsCh0;     // Channel 0 initial data
    const float* cellsCh1;     // Channel 1 initial data
    const float* cellsCh2;     // Channel 2 initial data
};

/// Get all single-channel presets
const std::vector<Preset>& getPresets();

/// Get list of preset category names for UI
const std::vector<std::string>& getPresetCategories();

/// Get kernel-only presets (for applying to existing simulations)
const std::vector<KernelPreset>& getKernelPresets();

/// Get multi-channel presets with cross-channel interactions
const std::vector<MultiChannelPreset>& getMultiChannelPresets();

}
