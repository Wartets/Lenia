#include "NpyLoader.hpp"
#include "Logger.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>

namespace lenia {

bool loadNpy(const std::string& path, NpyArray& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("NpyLoader: cannot open %s", path.c_str());
        return false;
    }

    char magic[6];
    file.read(magic, 6);
    if (magic[0] != '\x93' || std::string(magic + 1, 4) != "NUMP") {
        LOG_ERROR("NpyLoader: invalid magic in %s", path.c_str());
        return false;
    }

    uint8_t majorVer, minorVer;
    file.read(reinterpret_cast<char*>(&majorVer), 1);
    file.read(reinterpret_cast<char*>(&minorVer), 1);

    uint32_t headerLen = 0;
    if (majorVer == 1) {
        uint16_t hl;
        file.read(reinterpret_cast<char*>(&hl), 2);
        headerLen = hl;
    } else {
        file.read(reinterpret_cast<char*>(&headerLen), 4);
    }

    std::string header(headerLen, '\0');
    file.read(&header[0], headerLen);

    bool isFortranOrder = false;
    if (header.find("'fortran_order': True") != std::string::npos ||
        header.find("\"fortran_order\": True") != std::string::npos) {
        isFortranOrder = true;
    }

    bool isFloat64 = header.find("'<f8'") != std::string::npos ||
                     header.find("\"<f8\"") != std::string::npos ||
                     header.find("'float64'") != std::string::npos;
    bool isFloat32 = header.find("'<f4'") != std::string::npos ||
                     header.find("\"<f4\"") != std::string::npos ||
                     header.find("'float32'") != std::string::npos;

    if (!isFloat64 && !isFloat32) {
        LOG_ERROR("NpyLoader: unsupported dtype in %s: %s", path.c_str(), header.c_str());
        return false;
    }

    auto shapeStart = header.find("(");
    auto shapeEnd   = header.find(")");
    if (shapeStart == std::string::npos || shapeEnd == std::string::npos) {
        LOG_ERROR("NpyLoader: cannot parse shape in %s", path.c_str());
        return false;
    }

    std::string shapeStr = header.substr(shapeStart + 1, shapeEnd - shapeStart - 1);
    shapeStr.erase(std::remove(shapeStr.begin(), shapeStr.end(), ' '), shapeStr.end());

    std::vector<int> dims;
    size_t pos = 0;
    while (pos < shapeStr.size()) {
        size_t comma = shapeStr.find(',', pos);
        std::string token = shapeStr.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
        if (!token.empty()) {
            dims.push_back(std::stoi(token));
        }
        if (comma == std::string::npos) break;
        pos = comma + 1;
    }

    if (dims.size() == 2) {
        out.rows = dims[0];
        out.cols = dims[1];
    } else if (dims.size() == 1) {
        out.rows = 1;
        out.cols = dims[0];
    } else if (dims.size() == 3) {
        out.rows = dims[1];
        out.cols = dims[2];
        LOG_WARN("NpyLoader: 3D array in %s, using first channel (%dx%d)", path.c_str(), out.rows, out.cols);
    } else {
        LOG_ERROR("NpyLoader: unsupported %d-dim array in %s", static_cast<int>(dims.size()), path.c_str());
        return false;
    }

    int totalElements = 1;
    for (int d : dims) totalElements *= d;

    out.data.resize(out.rows * out.cols);

    if (isFloat64) {
        std::vector<double> raw(totalElements);
        file.read(reinterpret_cast<char*>(raw.data()), totalElements * sizeof(double));
        int slice = out.rows * out.cols;
        for (int i = 0; i < slice; ++i) {
            out.data[i] = static_cast<float>(raw[i]);
        }
    } else {
        std::vector<float> raw(totalElements);
        file.read(reinterpret_cast<char*>(raw.data()), totalElements * sizeof(float));
        int slice = out.rows * out.cols;
        for (int i = 0; i < slice; ++i) {
            out.data[i] = raw[i];
        }
    }

    if (isFortranOrder) {
        std::vector<float> transposed(out.rows * out.cols);
        for (int r = 0; r < out.rows; ++r)
            for (int c = 0; c < out.cols; ++c)
                transposed[r * out.cols + c] = out.data[c * out.rows + r];
        out.data = std::move(transposed);
    }

    LOG_INFO("NpyLoader: loaded %s (%dx%d)", path.c_str(), out.rows, out.cols);
    return true;
}

}
