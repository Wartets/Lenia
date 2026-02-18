/**
 * @file NpyLoader.hpp
 * @brief Loader for NumPy .npy files containing species patterns.
 * 
 * Supports float32 and float64 arrays in 1D, 2D, or 3D (first channel only).
 * Used to load pre-defined Lenia creature patterns.
 */

#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace lenia {

/**
 * @brief Container for loaded NPY array data.
 */
struct NpyArray {
    std::vector<float> data;  // Cell values (row-major order)
    int rows{0};              // Height of pattern
    int cols{0};              // Width of pattern
};

/**
 * @brief Load a NumPy .npy file into memory.
 * @param path Path to .npy file
 * @param out Output array structure
 * @return true on success, false on error
 */
bool loadNpy(const std::string& path, NpyArray& out);

}
