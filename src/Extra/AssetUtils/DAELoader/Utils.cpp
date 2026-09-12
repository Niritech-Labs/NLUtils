#include "../../../../include/NLUtils/Extra/AssetsUtils.hpp"
#include <sstream>
#include <functional>

bool COLLADALoader::VertexKey::operator==(const VertexKey& other) const {
    return pos == other.pos && normal == other.normal && uv == other.uv && jointIndices == other.jointIndices && jointWeights == other.jointWeights;
}

size_t COLLADALoader::VertexKeyHash::operator()(const VertexKey& k) const {
    size_t h1 = std::hash<float>()(k.pos.x) ^ (std::hash<float>()(k.pos.y) << 1) ^ (std::hash<float>()(k.pos.z) << 2);
    size_t h2 = std::hash<float>()(k.normal.x) ^ (std::hash<float>()(k.normal.y) << 1) ^ (std::hash<float>()(k.normal.z) << 2);
    size_t h3 = std::hash<float>()(k.uv.x) ^ (std::hash<float>()(k.uv.y) << 1);
    size_t h4 = std::hash<int>()(k.jointIndices.x) ^ (std::hash<int>()(k.jointIndices.y) << 1) ^ (std::hash<int>()(k.jointIndices.z) << 2) ^ (std::hash<int>()(k.jointIndices.w) << 3);
    size_t h5 = std::hash<float>()(k.jointWeights.x) ^ (std::hash<float>()(k.jointWeights.y) << 1) ^ (std::hash<float>()(k.jointWeights.z) << 2) ^ (std::hash<float>()(k.jointWeights.w) << 3);
    return h1 ^ h2 ^ h3 ^ h4 ^ h5;
}