#include "../../../../include/NLUtils/Extra/AssetsUtils.hpp"
#include <sstream>

void COLLADALoader::Setup(NLLogger* logger, bool production) {
    Logger = logger;
    xmlm.Setup("", production);
}

Model COLLADALoader::LoadModel(const std::string path) {
    pugi::xml_document doc = xmlm.OpenRestricted(path);
    Model model;
    if (doc.empty()) {
        Logger->Error("Failed to load COLLADA file: " + path, false, "ColladaLoader");
        return model;
    }

    auto libGeom = doc.child("COLLADA").child("library_geometries");
    if (!libGeom) {
        Logger->Error("No library_geometries found.", false, "ColladaLoader");
        return model;
    }

    std::vector<TempVertex> allVertices;
    std::vector<int> allRawIndices;
    std::vector<SubMesh> allSubMeshes;
    std::unordered_map<std::string, int> materialIndexMap;

    for (auto geometry : libGeom.children("geometry")) {
        auto meshNode = geometry.child("mesh");
        if (!meshNode) continue;

        std::vector<TempVertex> geomVerts;
        std::vector<int> geomRawIndices;
        std::vector<SubMesh> geomSubs;
        if (!parseStaticGeometry(meshNode, geomVerts, geomRawIndices, geomSubs, materialIndexMap)) {
            continue;
        }

        std::string geomId = geometry.attribute("id").as_string();
        auto libControllers = doc.child("COLLADA").child("library_controllers");
        pugi::xml_node skinNode;
        for (auto ctrl : libControllers.children("controller")) {
            auto skin = ctrl.child("skin");
            if (skin) {
                std::string src = skin.attribute("source").as_string();
                if (!src.empty() && src[0] == '#') src.erase(0, 1);
                if (src == geomId) {
                    skinNode = skin;
                    break;
                }
            }
        }

        if (skinNode) {
            if (!parseSkinning(skinNode, model.skeleton, geomVerts)) {
                Logger->Warning("Failed to parse skinning for geometry: " + geomId, "ColladaLoader");
            } else {
                auto libScenes = doc.child("COLLADA").child("library_visual_scenes");
                auto sceneNode = libScenes.child("visual_scene");
                if (sceneNode) {
                    std::vector<int> parentIndices;
                    std::vector<glm::mat4> localBindPoses;
                    if (parseSkeletonHierarchy(sceneNode, model.skeleton.jointNames, parentIndices, localBindPoses)) {
                        model.skeleton.parentIndices = std::move(parentIndices);
                        model.skeleton.localBindPoses = std::move(localBindPoses);
                    } else {
                        Logger->Warning("Failed to parse skeleton hierarchy for geometry: " + geomId, "ColladaLoader");
                    }
                }
            }
        }

        int baseVertex = (int)allVertices.size();
        int baseRawIndex = (int)allRawIndices.size();
        for (auto& v : geomVerts) allVertices.push_back(v);
        for (auto idx : geomRawIndices) allRawIndices.push_back(idx + baseVertex);
        for (auto& sub : geomSubs) {
            sub.vertexStart += baseVertex;
            sub.rawIndexStart += baseRawIndex;
            allSubMeshes.push_back(sub);
        }
    }

    if (allVertices.empty()) {
        Logger->Error("No geometry loaded.", false, "ColladaLoader");
        return model;
    }

    std::unordered_map<VertexKey, int, VertexKeyHash> vertexCache;
    std::vector<unsigned int> finalIndices;
    finalIndices.reserve(allRawIndices.size());

    for (int rawIdx : allRawIndices) {
        const TempVertex& v = allVertices[rawIdx];
        VertexKey key{v.pos, v.normal, v.uv, v.jointIndices, v.jointWeights};
        auto it = vertexCache.find(key);
        if (it == vertexCache.end()) {
            int newIdx = (int)vertexCache.size();
            vertexCache[key] = newIdx;
            finalIndices.push_back(newIdx);
        } else {
            finalIndices.push_back(it->second);
        }
    }

    int vertexCount = (int)vertexCache.size();
    int vertexSize = 16 * sizeof(float);
    model.vertexBuffer.vertexCount = vertexCount;
    model.vertexBuffer.vertexSize = vertexSize;

    std::vector<float> flatData;
    flatData.reserve(vertexCount * 16);
    for (const auto& kv : vertexCache) {
        const VertexKey& key = kv.first;
        flatData.push_back(key.pos.x);
        flatData.push_back(key.pos.y);
        flatData.push_back(key.pos.z);
        flatData.push_back(key.normal.x);
        flatData.push_back(key.normal.y);
        flatData.push_back(key.normal.z);
        flatData.push_back(key.uv.x);
        flatData.push_back(key.uv.y);
        flatData.push_back((float)key.jointIndices.x);
        flatData.push_back((float)key.jointIndices.y);
        flatData.push_back((float)key.jointIndices.z);
        flatData.push_back((float)key.jointIndices.w);
        flatData.push_back(key.jointWeights.x);
        flatData.push_back(key.jointWeights.y);
        flatData.push_back(key.jointWeights.z);
        flatData.push_back(key.jointWeights.w);
    }
    model.vertexBuffer.data = std::move(flatData);

    VertexAttribute posAttr{VertexAttribute::POSITION, 3, 0, vertexSize};
    VertexAttribute normAttr{VertexAttribute::NORMAL, 3, 3 * sizeof(float), vertexSize};
    VertexAttribute uvAttr{VertexAttribute::TEXCOORD, 2, 6 * sizeof(float), vertexSize};
    VertexAttribute idxAttr{VertexAttribute::JOINT_INDICES, 4, 8 * sizeof(float), vertexSize};
    VertexAttribute wgtAttr{VertexAttribute::JOINT_WEIGHTS, 4, 12 * sizeof(float), vertexSize};
    model.vertexBuffer.attributes = {posAttr, normAttr, uvAttr, idxAttr, wgtAttr};

    model.indexBuffer.data = std::move(finalIndices);
    model.indexBuffer.count = (int)model.indexBuffer.data.size();

    for (auto& sub : allSubMeshes) {
        sub.indexStart = sub.rawIndexStart;
        sub.indexCount = sub.rawIndexCount;
    }
    model.subMeshes = std::move(allSubMeshes);

    auto libMaterials = doc.child("COLLADA").child("library_materials");
    if (libMaterials) {
        for (auto matNode : libMaterials.children("material")) {
            Material mat = parseMaterial(matNode);
            if (!mat.name.empty()) {
                model.materials[mat.name] = mat;
            }
        }
    }

    auto libAnim = doc.child("COLLADA").child("library_animations");
    if (libAnim) {
        model.animations = parseAnimations(libAnim);
    }

    return model;
}