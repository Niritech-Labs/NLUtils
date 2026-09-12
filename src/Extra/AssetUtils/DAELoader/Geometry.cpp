#include "../../../../include/NLUtils/Extra/AssetsUtils.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <functional>
#include <algorithm>
#include <cmath>
#include <vector>
#include <map>

static bool isEar(const std::vector<int>& polygon, size_t i, const std::vector<glm::vec2>& points) {
    size_t n = polygon.size();
    size_t prev = (i == 0) ? n - 1 : i - 1;
    size_t next = (i + 1) % n;
    int pi = polygon[prev], pj = polygon[i], pk = polygon[next];
    glm::vec2 a = points[pi], b = points[pj], c = points[pk];
    float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cross <= 0.0f) return false;
    for (size_t j = 0; j < n; ++j) {
        if (j == prev || j == i || j == next) continue;
        glm::vec2 p = points[polygon[j]];
        float d1 = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
        float d2 = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
        float d3 = (a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x);
        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        if (!hasNeg && !hasPos) return false;
    }
    return true;
}

static std::vector<int> earClipping(const std::vector<glm::vec2>& points2d) {
    std::vector<int> polygon(points2d.size());
    for (size_t i = 0; i < points2d.size(); ++i) polygon[i] = (int)i;
    std::vector<int> triangles;
    while (polygon.size() >= 3) {
        bool found = false;
        for (size_t i = 0; i < polygon.size(); ++i) {
            if (isEar(polygon, i, points2d)) {
                size_t prev = (i == 0) ? polygon.size() - 1 : i - 1;
                size_t next = (i + 1) % polygon.size();
                triangles.push_back(polygon[prev]);
                triangles.push_back(polygon[i]);
                triangles.push_back(polygon[next]);
                polygon.erase(polygon.begin() + i);
                found = true;
                break;
            }
        }
        if (!found) {
            for (size_t i = 1; i + 1 < polygon.size(); ++i) {
                triangles.push_back(polygon[0]);
                triangles.push_back(polygon[i]);
                triangles.push_back(polygon[i + 1]);
            }
            break;
        }
    }
    return triangles;
}

COLLADALoader::ParsedSource COLLADALoader::parseSource(const pugi::xml_node& sourceNode) {
    COLLADALoader::ParsedSource result;
    auto floatArray = sourceNode.child("float_array");
    if (!floatArray) return result;
    std::stringstream ss(floatArray.text().as_string());
    float val;
    while (ss >> val) result.data.push_back(val);
    auto accessor = sourceNode.child("technique_common").child("accessor");
    if (accessor) {
        result.count = accessor.attribute("count").as_int();
        result.stride = accessor.attribute("stride").as_int(1);
    } else {
        result.count = (int)result.data.size();
        result.stride = 1;
    }
    return result;
}

bool COLLADALoader::parseStaticGeometry(const pugi::xml_node& meshNode, std::vector<TempVertex>& outVertices, std::vector<int>& outRawIndices, std::vector<SubMesh>& outSubMeshes, std::unordered_map<std::string, int>& materialIndexMap) {
    std::unordered_map<std::string, ParsedSource> sources;
    for (auto src : meshNode.children("source")) {
        std::string id = src.attribute("id").as_string();
        if (!id.empty()) sources[id] = parseSource(src);
    }

    for (auto prim : meshNode.children()) {
        std::string primName = prim.name();
        if (primName != "triangles" && primName != "polylist" && primName != "polygons") continue;
        std::string materialSymbol = prim.attribute("material").as_string();

        std::vector<VertexInputBinding> bindings;
        for (auto input : prim.children("input")) {
            VertexInputBinding b;
            b.offset = input.attribute("offset").as_int();
            b.semantic = input.attribute("semantic").as_string();
            b.sourceId = input.attribute("source").as_string();
            if (!b.sourceId.empty() && b.sourceId[0] == '#') b.sourceId.erase(0, 1);
            bindings.push_back(b);
        }

        int offsetPos = -1, offsetNorm = -1, offsetTex = -1;
        std::string srcPos, srcNorm, srcTex;

        for (auto& b : bindings) {
            if (b.semantic == "VERTEX") {
                auto verts = meshNode.child("vertices");
                if (verts && std::string(verts.attribute("id").as_string()) == b.sourceId) {
                    auto posInput = verts.child("input");
                    if (posInput && std::string(posInput.attribute("semantic").as_string()) == "POSITION") {
                        std::string src = posInput.attribute("source").as_string();
                        if (!src.empty() && src[0] == '#') src.erase(0, 1);
                        srcPos = src;
                        offsetPos = b.offset;
                    }
                }
            }
        }

        for (auto& b : bindings) {
            if (b.semantic == "POSITION") {
                srcPos = b.sourceId;
                offsetPos = b.offset;
            } else if (b.semantic == "NORMAL") {
                srcNorm = b.sourceId;
                offsetNorm = b.offset;
            } else if (b.semantic == "TEXCOORD") {
                srcTex = b.sourceId;
                offsetTex = b.offset;
            }
        }

        if (offsetPos == -1 || srcPos.empty()) {
            if (Logger) Logger->Warning("Primitive without POSITION, skipping", "ColladaLoader");
            continue;
        }

        int numInputs = (int)bindings.size();

        if (primName == "triangles") {
            auto pNode = prim.child("p");
            if (!pNode) continue;
            std::stringstream pss(pNode.text().as_string());
            std::vector<int> idxs;
            int idx;
            while (pss >> idx) idxs.push_back(idx);
            if (idxs.empty() || idxs.size() % numInputs != 0) continue;

            int startVertex = (int)outVertices.size();
            int rawIndexStart = (int)outRawIndices.size();

            for (size_t i = 0; i < idxs.size(); i += numInputs) {
                int posIdx = -1, normIdx = -1, texIdx = -1;
                for (auto& b : bindings) {
                    int curIdx = idxs[i + b.offset];
                    if (b.offset == offsetPos) posIdx = curIdx;
                    else if (b.offset == offsetNorm) normIdx = curIdx;
                    else if (b.offset == offsetTex) texIdx = curIdx;
                }
                if (posIdx < 0) continue;

                TempVertex v;
                v.pos = getVec3(sources[srcPos], posIdx);
                v.normal = glm::vec3(0,1,0);
                if (offsetNorm != -1 && !srcNorm.empty() && normIdx >= 0 && normIdx < sources[srcNorm].count)
                    v.normal = getVec3(sources[srcNorm], normIdx);
                v.uv = glm::vec2(0,0);
                if (offsetTex != -1 && !srcTex.empty() && texIdx >= 0 && texIdx < sources[srcTex].count)
                    v.uv = getVec2(sources[srcTex], texIdx);
                outVertices.push_back(v);
            }
            int vertexCount = (int)outVertices.size() - startVertex;
            if (vertexCount == 0) continue;
            for (int i = 0; i < vertexCount; ++i)
                outRawIndices.push_back(startVertex + i);

            SubMesh sub;
            sub.materialName = materialSymbol;
            sub.vertexStart = startVertex;
            sub.vertexCount = vertexCount;
            sub.rawIndexStart = rawIndexStart;
            sub.rawIndexCount = vertexCount;
            sub.indexStart = rawIndexStart;
            sub.indexCount = vertexCount;
            outSubMeshes.push_back(sub);
            continue;
        }

        if (primName == "polylist" || primName == "polygons") {
            auto pNode = prim.child("p");
            if (!pNode) continue;
            std::stringstream pss(pNode.text().as_string());
            std::vector<int> idxs;
            int idx;
            while (pss >> idx) idxs.push_back(idx);

            std::vector<int> vcounts;
            if (primName == "polylist") {
                auto vcountNode = prim.child("vcount");
                if (!vcountNode) continue;
                std::stringstream vss(vcountNode.text().as_string());
                int vc;
                while (vss >> vc) vcounts.push_back(vc);
            } else {
                if (idxs.empty()) continue;
                vcounts.push_back((int)idxs.size() / numInputs);
            }

            int offset = 0;
            for (int polyIdx = 0; polyIdx < (int)vcounts.size(); ++polyIdx) {
                int vertCount = vcounts[polyIdx];
                if (vertCount < 3) {
                    offset += vertCount * numInputs;
                    continue;
                }

                std::vector<AttributeIndices> polyAttrs;
                for (int i = 0; i < vertCount; ++i) {
                    int base = offset + i * numInputs;
                    AttributeIndices attr;
                    attr.posIdx = -1;
                    attr.normIdx = -1;
                    attr.texIdx = -1;
                    for (auto& b : bindings) {
                        int curIdx = idxs[base + b.offset];
                        if (b.offset == offsetPos) attr.posIdx = curIdx;
                        else if (b.offset == offsetNorm) attr.normIdx = curIdx;
                        else if (b.offset == offsetTex) attr.texIdx = curIdx;
                    }
                    if (attr.posIdx >= 0) polyAttrs.push_back(attr);
                }
                offset += vertCount * numInputs;
                if (polyAttrs.size() < 3) continue;

                glm::vec3 normal(0,1,0);
                if (polyAttrs.size() >= 3) {
                    glm::vec3 p0 = getVec3(sources[srcPos], polyAttrs[0].posIdx);
                    glm::vec3 p1 = getVec3(sources[srcPos], polyAttrs[1].posIdx);
                    glm::vec3 p2 = getVec3(sources[srcPos], polyAttrs[2].posIdx);
                    normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
                    if (glm::length(normal) < 1e-6f) normal = glm::vec3(0,1,0);
                }

                glm::vec3 u, v;
                if (fabs(normal.x) < fabs(normal.y) && fabs(normal.x) < fabs(normal.z))
                    u = glm::normalize(glm::cross(normal, glm::vec3(1,0,0)));
                else if (fabs(normal.y) < fabs(normal.x) && fabs(normal.y) < fabs(normal.z))
                    u = glm::normalize(glm::cross(normal, glm::vec3(0,1,0)));
                else
                    u = glm::normalize(glm::cross(normal, glm::vec3(0,0,1)));
                v = glm::normalize(glm::cross(normal, u));

                std::vector<glm::vec2> pts2d;
                for (auto& attr : polyAttrs) {
                    glm::vec3 p = getVec3(sources[srcPos], attr.posIdx);
                    pts2d.push_back(glm::vec2(glm::dot(p, u), glm::dot(p, v)));
                }

                std::vector<int> triIndices = earClipping(pts2d);
                if (triIndices.empty()) continue;

                int startVertex = (int)outVertices.size();
                for (int localIdx : triIndices) {
                    const AttributeIndices& attr = polyAttrs[localIdx];
                    TempVertex tv;
                    tv.pos = getVec3(sources[srcPos], attr.posIdx);
                    if (offsetNorm != -1 && !srcNorm.empty() && attr.normIdx >= 0 && attr.normIdx < sources[srcNorm].count)
                        tv.normal = getVec3(sources[srcNorm], attr.normIdx);
                    else
                        tv.normal = normal;
                    if (offsetTex != -1 && !srcTex.empty() && attr.texIdx >= 0 && attr.texIdx < sources[srcTex].count)
                        tv.uv = getVec2(sources[srcTex], attr.texIdx);
                    else
                        tv.uv = glm::vec2(0,0);
                    outVertices.push_back(tv);
                }

                int triCount = (int)triIndices.size();
                for (int t = 0; t < triCount; ++t)
                    outRawIndices.push_back(startVertex + t);

                SubMesh sub;
                sub.materialName = materialSymbol;
                sub.vertexStart = startVertex;
                sub.vertexCount = triCount;
                sub.rawIndexStart = (int)outRawIndices.size() - triCount;
                sub.rawIndexCount = triCount;
                sub.indexStart = sub.rawIndexStart;
                sub.indexCount = triCount;
                outSubMeshes.push_back(sub);
            }
            continue;
        }
    }
    return !outVertices.empty();
}

bool COLLADALoader::parseSkinning(const pugi::xml_node& skinNode, Skeleton& outSkeleton, std::vector<TempVertex>& outVertices) {
    std::string jointSrc, invBindSrc, weightSrc;

    auto jointsNode = skinNode.child("joints");
    if (!jointsNode) return false;
    for (auto input : jointsNode.children("input")) {
        std::string sem = input.attribute("semantic").as_string();
        std::string src = input.attribute("source").as_string();
        if (!src.empty() && src[0] == '#') src.erase(0, 1);
        if (sem == "JOINT") jointSrc = src;
        else if (sem == "INV_BIND_MATRIX") invBindSrc = src;
    }

    auto vwNode = skinNode.child("vertex_weights");
    if (!vwNode) return false;
    for (auto input : vwNode.children("input")) {
        std::string sem = input.attribute("semantic").as_string();
        std::string src = input.attribute("source").as_string();
        if (!src.empty() && src[0] == '#') src.erase(0, 1);
        if (sem == "WEIGHT") weightSrc = src;
    }

    auto srcJoint = skinNode.child("source").find_child_by_attribute("id", jointSrc.c_str());
    if (!srcJoint) return false;
    auto nameArr = srcJoint.child("Name_array");
    if (!nameArr) return false;
    std::stringstream namesStream(nameArr.text().as_string());
    std::string name;
    while (namesStream >> name) outSkeleton.jointNames.push_back(name);

    auto srcInv = skinNode.child("source").find_child_by_attribute("id", invBindSrc.c_str());
    if (!srcInv) return false;
    auto invArr = srcInv.child("float_array");
    if (!invArr) return false;
    std::stringstream invStream(invArr.text().as_string());
    float val;
    std::vector<float> invData;
    while (invStream >> val) invData.push_back(val);
    size_t numJoints = outSkeleton.jointNames.size();
    outSkeleton.inverseBindPoses.resize(numJoints);
    for (size_t i = 0; i < numJoints; ++i) {
        glm::mat4 mat;
        int base = i * 16;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                mat[c][r] = invData[base + r*4 + c];
        outSkeleton.inverseBindPoses[i] = mat;
    }

    auto srcW = skinNode.child("source").find_child_by_attribute("id", weightSrc.c_str());
    if (!srcW) return false;
    auto wArr = srcW.child("float_array");
    if (!wArr) return false;
    std::stringstream wStream(wArr.text().as_string());
    std::vector<float> weights;
    while (wStream >> val) weights.push_back(val);

    int vertexCount = vwNode.attribute("count").as_int();
    auto vcountNode = vwNode.child("vcount");
    auto vNode = vwNode.child("v");
    if (!vcountNode || !vNode) return false;

    std::stringstream vcountStream(vcountNode.text().as_string());
    std::vector<int> vcounts;
    int cnt;
    while (vcountStream >> cnt) vcounts.push_back(cnt);

    std::stringstream vStream(vNode.text().as_string());
    std::vector<int> vData;
    while (vStream >> cnt) vData.push_back(cnt);

    if ((int)outVertices.size() != vertexCount) {
        if (Logger) Logger->Error("Vertex count mismatch in skinning.", false, "ColladaLoader");
        return false;
    }

    int vOffset = 0;
    for (int v = 0; v < vertexCount; ++v) {
        int numInf = vcounts[v];
        std::vector<std::pair<int, float>> infs;
        for (int i = 0; i < numInf; ++i) {
            int jIdx = vData[vOffset++];
            int wIdx = vData[vOffset++];
            float w = weights[wIdx];
            if (w > 0.0f) infs.push_back({jIdx, w});
        }
        std::sort(infs.begin(), infs.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
        if (infs.size() > 4) infs.resize(4);
        float sum = 0.0f;
        for (auto& inf : infs) sum += inf.second;
        if (sum > 0.0f) {
            for (auto& inf : infs) inf.second /= sum;
        }
        glm::ivec4 indices(0);
        glm::vec4 weights4(0.0f);
        for (size_t i = 0; i < infs.size(); ++i) {
            indices[i] = infs[i].first;
            weights4[i] = infs[i].second;
        }
        outVertices[v].jointIndices = indices;
        outVertices[v].jointWeights = weights4;
    }

    outSkeleton.localBindPoses.resize(numJoints, glm::mat4(1.0f));
    outSkeleton.parentIndices.assign(numJoints, -1);
    return true;
}

bool COLLADALoader::parseSkeletonHierarchy(const pugi::xml_node& visualSceneNode, const std::vector<std::string>& jointNames, std::vector<int>& parentIndices, std::vector<glm::mat4>& localBindPoses) {
    parentIndices.assign(jointNames.size(), -1);
    localBindPoses.resize(jointNames.size());

    std::unordered_map<std::string, int> nameToIndex;
    for (int i = 0; i < (int)jointNames.size(); ++i)
        nameToIndex[jointNames[i]] = i;

    std::function<void(const pugi::xml_node&, int)> traverse = [&](const pugi::xml_node& node, int parentIdx) {
        std::string sid = node.attribute("sid").as_string();
        if (node.attribute("type").as_string() == "JOINT" && !sid.empty()) {
            auto it = nameToIndex.find(sid);
            if (it == nameToIndex.end()) {
                std::string name = node.attribute("name").as_string();
                if (!name.empty()) it = nameToIndex.find(name);
            }
            if (it == nameToIndex.end()) {
                std::string id = node.attribute("id").as_string();
                if (!id.empty()) it = nameToIndex.find(id);
            }
            if (it != nameToIndex.end()) {
                int idx = it->second;
                parentIndices[idx] = parentIdx;

                glm::mat4 local = glm::mat4(1.0f);
                for (auto child : node.children()) {
                    std::string name = child.name();
                    if (name == "translate") {
                        std::stringstream ss(child.text().as_string());
                        glm::vec3 t; ss >> t.x >> t.y >> t.z;
                        local = glm::translate(local, t);
                    } else if (name == "rotate") {
                        std::stringstream ss(child.text().as_string());
                        glm::vec3 axis; float angleDeg;
                        ss >> axis.x >> axis.y >> axis.z >> angleDeg;
                        local = glm::rotate(local, glm::radians(angleDeg), axis);
                    } else if (name == "scale") {
                        std::stringstream ss(child.text().as_string());
                        glm::vec3 s; ss >> s.x >> s.y >> s.z;
                        local = glm::scale(local, s);
                    } else if (name == "matrix") {
                        std::stringstream ss(child.text().as_string());
                        float m[16];
                        for (int i=0; i<16; ++i) ss >> m[i];
                        local = glm::make_mat4(m);
                    }
                }
                localBindPoses[idx] = local;
                parentIdx = idx;
            }
        }
        for (auto child : node.children("node")) {
            traverse(child, parentIdx);
        }
    };

    for (auto node : visualSceneNode.children("node")) {
        traverse(node, -1);
    }
    return true;
}

glm::vec3 COLLADALoader::getVec3(const ParsedSource& src, int idx) {
    int start = idx * src.stride;
    if (start + 2 >= (int)src.data.size()) return glm::vec3(0.0f);
    return glm::vec3(src.data[start], src.data[start+1], src.data[start+2]);
}

glm::vec2 COLLADALoader::getVec2(const ParsedSource& src, int idx) {
    int start = idx * src.stride;
    if (start + 1 >= (int)src.data.size()) return glm::vec2(0.0f);
    return glm::vec2(src.data[start], src.data[start+1]);
}