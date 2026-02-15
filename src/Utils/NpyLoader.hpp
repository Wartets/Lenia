#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace lenia {

struct NpyArray {
    std::vector<float> data;
    int rows{0};
    int cols{0};
};

bool loadNpy(const std::string& path, NpyArray& out);

}
