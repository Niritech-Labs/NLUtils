#include "../../../include/NLUtils/Extra/AssetsUtils.hpp"

GLModelObject* AssetManager::GetGLMO(ModelType modelType) {
    if (!GLInit) {
        Logger.Warning("GLAD is not initialized");
        return nullptr;
    }
    Logger.Info("Getting GLModel",NLLogger::GREEN,false,"GLModel");
    auto it = GLModelCache.find(modelType.GetHash());
    if (it != GLModelCache.end()) return it->second;
    GLModelObject* model = nullptr;
    if (modelType.type == ModelType::COLLADA) {
        model = loadCollada(modelType.data.ColladaPath);
    }
    if (modelType.type == ModelType::CAPSULE) {
        model = loadCapsule(modelType.data.CapsuleSize.x,modelType.data.CapsuleSize.y);
    }
    if (modelType.type == ModelType::CUBE) {
        model = loadCube(modelType.data.CubeSize);
    }
    if (modelType.type == ModelType::SPHERE) {
        model = loadSphere(modelType.data.SphereRadius.x);
    }
    if (modelType.type == ModelType::PLANE) {
        model = loadPlane(modelType.data.PlaneSize);
    }
    if (modelType.type == ModelType::DYNAMIC) {
        model = loadDynamicGLMO();
    }
    
    if (model) {
        GLModelCache[modelType.GetHash()] = model;
        GLModels.push_back(model);
    }
    return model;
    
}
Model* AssetManager::GetModel(ModelType modelType) {
    Logger.Info("Getting model", NLLogger::GREEN, false, "Model");
    auto it = modelCache.find(modelType.GetHash());
    if (it != modelCache.end()) return it->second;
    Model* model = nullptr;
    if (modelType.type == ModelType::COLLADA) {
        model = loadColladaBase(modelType.data.ColladaPath);
    }
    if (modelType.type == ModelType::CAPSULE) {
        model = loadCapsuleBase(modelType.data.CapsuleSize.x, modelType.data.CapsuleSize.y);
    }
    if (modelType.type == ModelType::CUBE) {
        model = loadCubeBase(modelType.data.CubeSize);
    }
    if (modelType.type == ModelType::SPHERE) {
        model = loadSphereBase(modelType.data.SphereRadius.x);
    }
    if (modelType.type == ModelType::PLANE) {
        model = loadPlaneBase(modelType.data.PlaneSize);
    }
    if (modelType.type == ModelType::DYNAMIC) {
        Logger.Warning("DYNAMIC ModelType is not supported for Model*", "Model");
        return nullptr;
    }
    
    if (model) {
        modelCache[modelType.GetHash()] = model;
        models.push_back(model);
    }
    return model;
}
Model* AssetManager::loadColladaBase(std::string path) {
    Logger.Info("Loading COLLADA Model (CPU): " + path, NLLogger::GREEN, false, "Model");
    Model* model = new Model();
    *model = ColladaLoader.LoadModel(path);
    if (model->vertexBuffer.vertexCount == 0) {
        Logger.Error("Failed to load COLLADA model: " + path, false, "Model");
        delete model;
        return nullptr;
    }
    Logger.Info("COLLADA Model loaded successfully: " + path, NLLogger::GREEN, false, "Model");
    return model;
}
Model* AssetManager::loadSphereBase(float radius) {
    Logger.Info("Generating sphere Model, radius: " + std::to_string(radius), NLLogger::GREEN, false, "Model");
    int seg = 32;
    struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    for (int j = 0; j <= seg; ++j) {
        float theta = (float)j / seg * 3.14159265f;
        float sinT = sin(theta), cosT = cos(theta);
        for (int i = 0; i <= seg; ++i) {
            float phi = (float)i / seg * 2.0f * 3.14159265f;
            float sinP = sin(phi), cosP = cos(phi);
            glm::vec3 p = radius * glm::vec3(sinT*cosP, cosT, sinT*sinP);
            verts.push_back({p, glm::normalize(p), {(float)i/seg, (float)j/seg}});
        }
    }
    for (int j = 0; j < seg; ++j) {
        for (int i = 0; i < seg; ++i) {
            int a = j*(seg+1)+i;
            int b = a+1;
            int c = (j+1)*(seg+1)+i;
            int d = c+1;
            idx.push_back(static_cast<unsigned int>(a));
            idx.push_back(static_cast<unsigned int>(c));
            idx.push_back(static_cast<unsigned int>(b));
            idx.push_back(static_cast<unsigned int>(b));
            idx.push_back(static_cast<unsigned int>(c));
            idx.push_back(static_cast<unsigned int>(d));
        }
    }

    Model* model = new Model();
    model->vertexBuffer.vertexCount = static_cast<int>(verts.size());
    model->vertexBuffer.vertexSize = static_cast<int>(sizeof(Vertex));
    model->vertexBuffer.data.resize(verts.size() * (3 + 3 + 2));
    float* out = model->vertexBuffer.data.data();
    for (const auto& v : verts) {
        *out++ = v.pos.x; *out++ = v.pos.y; *out++ = v.pos.z;
        *out++ = v.normal.x; *out++ = v.normal.y; *out++ = v.normal.z;
        *out++ = v.uv.x; *out++ = v.uv.y;
    }
    model->vertexBuffer.attributes.clear();
    model->vertexBuffer.attributes.push_back({VertexAttribute::POSITION, 3, 0, static_cast<int>(sizeof(Vertex))});
    model->vertexBuffer.attributes.push_back({VertexAttribute::NORMAL, 3, static_cast<int>(offsetof(Vertex, normal)), static_cast<int>(sizeof(Vertex))});
    model->vertexBuffer.attributes.push_back({VertexAttribute::TEXCOORD, 2, static_cast<int>(offsetof(Vertex, uv)), static_cast<int>(sizeof(Vertex))});

    model->indexBuffer.data = idx;
    model->indexBuffer.count = static_cast<int>(idx.size());

    model->subMeshes.clear();
    SubMesh sm;
    sm.materialName = "default";
    sm.vertexStart = 0;
    sm.vertexCount = static_cast<int>(verts.size());
    sm.indexStart = 0;
    sm.indexCount = static_cast<int>(idx.size());
    sm.rawIndexStart = 0;
    sm.rawIndexCount = static_cast<int>(idx.size());
    model->subMeshes.push_back(sm);

    model->materials.clear();
    Material mat;
    mat.name = "default";
    mat.diffuse = glm::vec3(0.8f);
    mat.roughness = 0.5f;
    mat.metalness = 0.0f;
    mat.opacity = 1.0f;
    model->materials["default"] = mat;

    model->skeleton.jointNames.clear();
    model->animations.clear();

    Logger.Info("Sphere Model generated", NLLogger::GREEN, false, "Model");
    return model;
}
Model* AssetManager::loadCubeBase(glm::vec3 size) {
    Logger.Info("Generating cube Model: " + std::to_string(size.x) + "x" + std::to_string(size.y) + "x" + std::to_string(size.z), NLLogger::GREEN, false, "Model");
    float hx = size.x/2, hy = size.y/2, hz = size.z/2;
    struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    auto addFace = [&](glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 norm) {
        int base = static_cast<int>(verts.size());
        verts.push_back({v0, norm, {0,0}});
        verts.push_back({v1, norm, {1,0}});
        verts.push_back({v2, norm, {1,1}});
        verts.push_back({v3, norm, {0,1}});
        idx.push_back(static_cast<unsigned int>(base + 0));
        idx.push_back(static_cast<unsigned int>(base + 1));
        idx.push_back(static_cast<unsigned int>(base + 2));
        idx.push_back(static_cast<unsigned int>(base + 0));
        idx.push_back(static_cast<unsigned int>(base + 2));
        idx.push_back(static_cast<unsigned int>(base + 3));
    };
    addFace({-hx,-hy, hz}, { hx,-hy, hz}, { hx, hy, hz}, {-hx, hy, hz}, {0,0,1});
    addFace({ hx,-hy,-hz}, {-hx,-hy,-hz}, {-hx, hy,-hz}, { hx, hy,-hz}, {0,0,-1});
    addFace({-hx,-hy,-hz}, {-hx,-hy, hz}, {-hx, hy, hz}, {-hx, hy,-hz}, {-1,0,0});
    addFace({ hx,-hy, hz}, { hx,-hy,-hz}, { hx, hy,-hz}, { hx, hy, hz}, {1,0,0});
    addFace({-hx,-hy,-hz}, { hx,-hy,-hz}, { hx,-hy, hz}, {-hx,-hy, hz}, {0,-1,0});
    addFace({-hx, hy, hz}, { hx, hy, hz}, { hx, hy,-hz}, {-hx, hy,-hz}, {0,1,0});

    Model* model = new Model();
    model->vertexBuffer.vertexCount = static_cast<int>(verts.size());
    model->vertexBuffer.vertexSize = static_cast<int>(sizeof(Vertex));
    model->vertexBuffer.data.resize(verts.size() * (3 + 3 + 2));
    float* out = model->vertexBuffer.data.data();
    for (const auto& v : verts) {
        *out++ = v.pos.x; *out++ = v.pos.y; *out++ = v.pos.z;
        *out++ = v.normal.x; *out++ = v.normal.y; *out++ = v.normal.z;
        *out++ = v.uv.x; *out++ = v.uv.y;
    }
    model->vertexBuffer.attributes.clear();
    model->vertexBuffer.attributes.push_back({VertexAttribute::POSITION, 3, 0, static_cast<int>(sizeof(Vertex))});
    model->vertexBuffer.attributes.push_back({VertexAttribute::NORMAL, 3, static_cast<int>(offsetof(Vertex, normal)), static_cast<int>(sizeof(Vertex))});
    model->vertexBuffer.attributes.push_back({VertexAttribute::TEXCOORD, 2, static_cast<int>(offsetof(Vertex, uv)), static_cast<int>(sizeof(Vertex))});

    model->indexBuffer.data = idx;
    model->indexBuffer.count = static_cast<int>(idx.size());

    model->subMeshes.clear();
    SubMesh sm;
    sm.materialName = "default";
    sm.vertexStart = 0;
    sm.vertexCount = static_cast<int>(verts.size());
    sm.indexStart = 0;
    sm.indexCount = static_cast<int>(idx.size());
    sm.rawIndexStart = 0;
    sm.rawIndexCount = static_cast<int>(idx.size());
    model->subMeshes.push_back(sm);

    model->materials.clear();
    Material mat;
    mat.name = "default";
    mat.diffuse = glm::vec3(0.8f);
    mat.roughness = 0.5f;
    mat.metalness = 0.0f;
    mat.opacity = 1.0f;
    model->materials["default"] = mat;

    model->skeleton.jointNames.clear();
    model->animations.clear();

    Logger.Info("Cube Model generated", NLLogger::GREEN, false, "Model");
    return model;
}
Model* AssetManager::loadPlaneBase(glm::vec2 size) {
    Logger.Info("Generating plane Model: " + std::to_string(size.x) + "x" + std::to_string(size.y), NLLogger::GREEN, false, "Model");
    float hw = size.x/2, hh = size.y/2;
    struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };
    std::vector<Vertex> verts = {
        {{-hw, 0, -hh}, {0,1,0}, {0,0}},
        {{ hw, 0, -hh}, {0,1,0}, {1,0}},
        {{ hw, 0,  hh}, {0,1,0}, {1,1}},
        {{-hw, 0,  hh}, {0,1,0}, {0,1}}
    };
    std::vector<unsigned int> idx = {0u, 1u, 2u, 0u, 2u, 3u};

    Model* model = new Model();
    model->vertexBuffer.vertexCount = static_cast<int>(verts.size());
    model->vertexBuffer.vertexSize = static_cast<int>(sizeof(Vertex));
    model->vertexBuffer.data.resize(verts.size() * (3 + 3 + 2));
    float* out = model->vertexBuffer.data.data();
    for (const auto& v : verts) {
        *out++ = v.pos.x; *out++ = v.pos.y; *out++ = v.pos.z;
        *out++ = v.normal.x; *out++ = v.normal.y; *out++ = v.normal.z;
        *out++ = v.uv.x; *out++ = v.uv.y;
    }
    model->vertexBuffer.attributes.clear();
    model->vertexBuffer.attributes.push_back({VertexAttribute::POSITION, 3, 0, static_cast<int>(sizeof(Vertex))});
    model->vertexBuffer.attributes.push_back({VertexAttribute::NORMAL, 3, static_cast<int>(offsetof(Vertex, normal)), static_cast<int>(sizeof(Vertex))});
    model->vertexBuffer.attributes.push_back({VertexAttribute::TEXCOORD, 2, static_cast<int>(offsetof(Vertex, uv)), static_cast<int>(sizeof(Vertex))});

    model->indexBuffer.data = idx;
    model->indexBuffer.count = static_cast<int>(idx.size());

    model->subMeshes.clear();
    SubMesh sm;
    sm.materialName = "default";
    sm.vertexStart = 0;
    sm.vertexCount = static_cast<int>(verts.size());
    sm.indexStart = 0;
    sm.indexCount = static_cast<int>(idx.size());
    sm.rawIndexStart = 0;
    sm.rawIndexCount = static_cast<int>(idx.size());
    model->subMeshes.push_back(sm);

    model->materials.clear();
    Material mat;
    mat.name = "default";
    mat.diffuse = glm::vec3(0.8f);
    mat.roughness = 0.5f;
    mat.metalness = 0.0f;
    mat.opacity = 1.0f;
    model->materials["default"] = mat;

    model->skeleton.jointNames.clear();
    model->animations.clear();

    Logger.Info("Plane Model generated", NLLogger::GREEN, false, "Model");
    return model;
}
Model* AssetManager::loadCapsuleBase(float radius, float height) {
    Logger.Info("Generating capsule Model, radius: " + std::to_string(radius) + ", height: " + std::to_string(height), NLLogger::GREEN, false, "Model");
    int seg = 32;
    float halfH = height / 2.0f;
    struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    auto addHemisphere = [&](float yOff, float sign) {
        for (int j = 0; j <= seg/2; ++j) {
            float theta = (float)j / (seg/2) * 3.14159265f / 2.0f;
            float sinT = sin(theta), cosT = cos(theta);
            float y = yOff + sign * radius * cosT;
            for (int i = 0; i <= seg; ++i) {
                float phi = (float)i / seg * 2.0f * 3.14159265f;
                float sinP = sin(phi), cosP = cos(phi);
                glm::vec3 p = radius * glm::vec3(sinT*cosP, sign*cosT, sinT*sinP);
                p.y += yOff;
                verts.push_back({p, glm::normalize(p - glm::vec3(0,yOff,0)), {(float)i/seg, (float)j/(seg/2)}});
            }
        }
    };
    addHemisphere(halfH, 1.0f);
    addHemisphere(-halfH, -1.0f);

    for (int j = 0; j <= seg/2; ++j) {
        float y = -halfH + (float)j / (seg/2) * height;
        float t = (float)j / (seg/2);
        for (int i = 0; i <= seg; ++i) {
            float phi = (float)i / seg * 2.0f * 3.14159265f;
            float sinP = sin(phi), cosP = cos(phi);
            glm::vec3 p = radius * glm::vec3(cosP, 0, sinP);
            p.y = y;
            verts.push_back({p, glm::normalize(glm::vec3(cosP, 0, sinP)), {(float)i/seg, t}});
        }
    }

    int cylBase = (seg/2 + 1);
    for (int j = 0; j < seg/2; ++j) {
        for (int i = 0; i < seg; ++i) {
            int a = j*(seg+1)+i;
            int b = a+1;
            int c = (j+1)*(seg+1)+i;
            int d = c+1;
            idx.push_back(static_cast<unsigned int>(a));
            idx.push_back(static_cast<unsigned int>(c));
            idx.push_back(static_cast<unsigned int>(b));
            idx.push_back(static_cast<unsigned int>(b));
            idx.push_back(static_cast<unsigned int>(c));
            idx.push_back(static_cast<unsigned int>(d));
        }
    }
    for (int j = 0; j < seg/2; ++j) {
        for (int i = 0; i < seg; ++i) {
            int a = cylBase + j*(seg+1)+i;
            int b = a+1;
            int c = cylBase + (j+1)*(seg+1)+i;
            int d = c+1;
            idx.push_back(static_cast<unsigned int>(a));
            idx.push_back(static_cast<unsigned int>(c));
            idx.push_back(static_cast<unsigned int>(b));
            idx.push_back(static_cast<unsigned int>(b));
            idx.push_back(static_cast<unsigned int>(c));
            idx.push_back(static_cast<unsigned int>(d));
        }
    }
    int bottomBase = cylBase + (seg/2)*(seg+1);
    for (int j = 0; j < seg/2; ++j) {
        for (int i = 0; i < seg; ++i) {
            int a = bottomBase + j*(seg+1)+i;
            int b = a+1;
            int c = bottomBase + (j+1)*(seg+1)+i;
            int d = c+1;
            idx.push_back(static_cast<unsigned int>(a));
            idx.push_back(static_cast<unsigned int>(c));
            idx.push_back(static_cast<unsigned int>(b));
            idx.push_back(static_cast<unsigned int>(b));
            idx.push_back(static_cast<unsigned int>(c));
            idx.push_back(static_cast<unsigned int>(d));
        }
    }

    Model* model = new Model();
    model->vertexBuffer.vertexCount = static_cast<int>(verts.size());
    model->vertexBuffer.vertexSize = static_cast<int>(sizeof(Vertex));
    model->vertexBuffer.data.resize(verts.size() * (3 + 3 + 2));
    float* out = model->vertexBuffer.data.data();
    for (const auto& v : verts) {
        *out++ = v.pos.x; *out++ = v.pos.y; *out++ = v.pos.z;
        *out++ = v.normal.x; *out++ = v.normal.y; *out++ = v.normal.z;
        *out++ = v.uv.x; *out++ = v.uv.y;
    }
    model->vertexBuffer.attributes.clear();
    model->vertexBuffer.attributes.push_back({VertexAttribute::POSITION, 3, 0, static_cast<int>(sizeof(Vertex))});
    model->vertexBuffer.attributes.push_back({VertexAttribute::NORMAL, 3, static_cast<int>(offsetof(Vertex, normal)), static_cast<int>(sizeof(Vertex))});
    model->vertexBuffer.attributes.push_back({VertexAttribute::TEXCOORD, 2, static_cast<int>(offsetof(Vertex, uv)), static_cast<int>(sizeof(Vertex))});

    model->indexBuffer.data = idx;
    model->indexBuffer.count = static_cast<int>(idx.size());

    model->subMeshes.clear();
    SubMesh sm;
    sm.materialName = "default";
    sm.vertexStart = 0;
    sm.vertexCount = static_cast<int>(verts.size());
    sm.indexStart = 0;
    sm.indexCount = static_cast<int>(idx.size());
    sm.rawIndexStart = 0;
    sm.rawIndexCount = static_cast<int>(idx.size());
    model->subMeshes.push_back(sm);

    model->materials.clear();
    Material mat;
    mat.name = "default";
    mat.diffuse = glm::vec3(0.8f);
    mat.roughness = 0.5f;
    mat.metalness = 0.0f;
    mat.opacity = 1.0f;
    model->materials["default"] = mat;

    model->skeleton.jointNames.clear();
    model->animations.clear();

    Logger.Info("Capsule Model generated", NLLogger::GREEN, false, "Model");
    return model;
}


GLModelObject* AssetManager::loadCollada(std::string path) {
    Logger.Info("Loading COLLADA: " + path, NLLogger::GREEN, false, "GLModel");
    Model model = ColladaLoader.LoadModel(path);
    if (model.vertexBuffer.vertexCount == 0) {
        Logger.Error("Failed to load COLLADA: " + path, false, "GLModel");
        return nullptr;
    }

    GLModelObject* obj = new GLModelObject();
    const VertexBuffer& vb = model.vertexBuffer;
    const IndexBuffer& ib = model.indexBuffer;

    glGenVertexArrays(1, &obj->vao);
    glGenBuffers(1, &obj->vbo);
    glGenBuffers(1, &obj->ibo);
    glBindVertexArray(obj->vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, vb.data.size() * sizeof(float), vb.data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ib.data.size() * sizeof(unsigned int), ib.data.data(), GL_STATIC_DRAW);
    obj->vertexCount = vb.vertexCount;
    obj->indexCount = ib.count;

    obj->vertexAttributes.clear();
    int attrIdx = 0;
    for (const auto& attr : vb.attributes) {
        GLVertexAttribute gattr;
        gattr.location = attrIdx;
        gattr.size = attr.components;
        gattr.type = GL_FLOAT;
        gattr.normalized = GL_FALSE;
        gattr.stride = vb.vertexSize;
        gattr.offset = attr.offset;
        obj->vertexAttributes.push_back(gattr);
        glEnableVertexAttribArray(attrIdx);
        glVertexAttribPointer(attrIdx, attr.components, GL_FLOAT, GL_FALSE, vb.vertexSize, reinterpret_cast<void*>(static_cast<uintptr_t>(attr.offset)));
        attrIdx++;
    }
    glBindVertexArray(0);

    obj->subMeshes.clear();
    std::vector<Material> matList;
    matList.reserve(model.materials.size());
    for (const auto& pair : model.materials) matList.push_back(pair.second);
    for (const auto& sm : model.subMeshes) {
        GLSubMesh gsm;
        gsm.vertexOffset = sm.vertexStart;
        gsm.indexOffset = sm.indexStart;
        gsm.indexCount = sm.indexCount;
        int matIdx = 0;
        for (size_t i = 0; i < matList.size(); ++i) {
            if (matList[i].name == sm.materialName) { matIdx = (int)i; break; }
        }
        gsm.materialIndex = matIdx;
        gsm.primitiveType = GL_TRIANGLES;
        obj->subMeshes.push_back(gsm);
    }

    obj->materials.clear();
    for (const auto& mat : matList) {
        GLMaterial gmat;
        if (!mat.diffuseMap.empty())   gmat.diffuseMap = LoadTexture(mat.diffuseMap);
        if (!mat.normalMap.empty())    gmat.normalMap = LoadTexture(mat.normalMap);
        if (!mat.specularMap.empty())  gmat.specularMap = LoadTexture(mat.specularMap);
        if (!mat.emissiveMap.empty())  gmat.emissiveMap = LoadTexture(mat.emissiveMap);
        gmat.baseColor = glm::vec4(mat.diffuse, 1.0f);
        gmat.roughness = mat.roughness;
        gmat.metalness = mat.metalness;
        gmat.emissiveIntensity = mat.emission;
        gmat.opacity = mat.opacity;
        gmat.translucent = (mat.opacity < 1.0f);
        obj->materials.push_back(gmat);
    }

    if (!model.skeleton.jointNames.empty()) {
        obj->hasSkeleton = true;
        obj->skeleton.jointCount = (GLuint)model.skeleton.jointNames.size();
        glGenBuffers(1, &obj->skeleton.jointBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, obj->skeleton.jointBuffer);
        glBufferData(GL_UNIFORM_BUFFER, obj->skeleton.jointCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        if (!model.skeleton.inverseBindPoses.empty()) {
            glGenBuffers(1, &obj->skeleton.inverseBindBuffer);
            glBindBuffer(GL_UNIFORM_BUFFER, obj->skeleton.inverseBindBuffer);
            glBufferData(GL_UNIFORM_BUFFER, model.skeleton.inverseBindPoses.size() * sizeof(glm::mat4),model.skeleton.inverseBindPoses.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
    } else {
        obj->hasSkeleton = false;
        obj->skeleton.jointBuffer = 0;
        obj->skeleton.jointCount = 0;
        obj->skeleton.inverseBindBuffer = 0;
    }

    obj->animations = model.animations;

    glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
    int posOffset = 0;
    bool foundPos = false;
    for (const auto& attr : vb.attributes) {
        if (attr.type == VertexAttribute::POSITION) {
            posOffset = attr.offset;
            foundPos = true;
            break;
        }
    }
    if (foundPos) {
        const float* data = vb.data.data();
        for (int i = 0; i < vb.vertexCount; ++i) {
            const float* pos = (const float*)((const char*)data + i * vb.vertexSize + posOffset);
            glm::vec3 p(pos[0], pos[1], pos[2]);
            minPos = glm::min(minPos, p);
            maxPos = glm::max(maxPos, p);
        }
    } else {
        minPos = glm::vec3(0.0f);
        maxPos = glm::vec3(0.0f);
    }
    obj->boundingMin = minPos;
    obj->boundingMax = maxPos;

    obj->instanceBuffer = 0;
    obj->instanceCount = 0;

    Logger.Info("COLLADA loaded successfully: " + path, NLLogger::GREEN, false, "GLModel");
    return obj;
}

GLModelObject* AssetManager::loadDynamicGLMO() {
    Logger.Info("Creating empty dynamic GLMO", NLLogger::GREEN, false, "GLModel");
    GLModelObject* obj = new GLModelObject();
    glGenVertexArrays(1, &obj->vao);
    glGenBuffers(1, &obj->vbo);
    glGenBuffers(1, &obj->ibo);
    obj->vertexCount = 0;
    obj->indexCount = 0;
    obj->vertexAttributes.clear();
    obj->subMeshes.clear();
    obj->materials.clear();
    GLMaterial mat;
    mat.diffuseMap = 0; mat.normalMap = 0; mat.specularMap = 0; mat.emissiveMap = 0;
    mat.baseColor = glm::vec4(1,1,1,1);
    mat.roughness = 0.5f; mat.metalness = 0.0f; mat.emissiveIntensity = 0.0f; mat.opacity = 1.0f;
    mat.translucent = false;
    obj->materials.push_back(mat);
    obj->hasSkeleton = false;
    obj->skeleton.jointBuffer = 0; obj->skeleton.jointCount = 0; obj->skeleton.inverseBindBuffer = 0;
    obj->animations.clear();
    obj->boundingMin = glm::vec3(0); obj->boundingMax = glm::vec3(0);
    obj->instanceBuffer = 0; obj->instanceCount = 0;
    Logger.Info("Empty GLMO created", NLLogger::GREEN, false, "GLModel");
    return obj;
}
GLModelObject* AssetManager::loadSphere(float radius) {
    Logger.Info("Generating sphere, radius: " + std::to_string(radius), NLLogger::GREEN, false, "GLModel");
    int seg = 32;
    struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
    for (int j = 0; j <= seg; ++j) {
        float theta = (float)j / seg * 3.14159265f;
        float sinT = sin(theta), cosT = cos(theta);
        for (int i = 0; i <= seg; ++i) {
            float phi = (float)i / seg * 2.0f * 3.14159265f;
            float sinP = sin(phi), cosP = cos(phi);
            glm::vec3 p = radius * glm::vec3(sinT*cosP, cosT, sinT*sinP);
            verts.push_back({p, glm::normalize(p), {(float)i/seg, (float)j/seg}});
            minPos = glm::min(minPos, p); maxPos = glm::max(maxPos, p);
        }
    }
    for (int j = 0; j < seg; ++j) {
        for (int i = 0; i < seg; ++i) {
            int a = j*(seg+1)+i, b = a+1, c = (j+1)*(seg+1)+i, d = c+1;
            idx.insert(idx.end(), {static_cast<unsigned int>(a), static_cast<unsigned int>(c), static_cast<unsigned int>(b), static_cast<unsigned int>(b), static_cast<unsigned int>(c), static_cast<unsigned int>(d)});
        }
    }
    GLModelObject* obj = new GLModelObject();
    glGenVertexArrays(1, &obj->vao);
    glGenBuffers(1, &obj->vbo);
    glGenBuffers(1, &obj->ibo);
    glBindVertexArray(obj->vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    obj->vertexCount = (GLsizei)verts.size();
    obj->indexCount = (GLsizei)idx.size();

    GLsizei stride = sizeof(Vertex);
    obj->vertexAttributes.clear();
    obj->vertexAttributes.push_back({0,3,GL_FLOAT,GL_FALSE,stride,0});
    obj->vertexAttributes.push_back({1,3,GL_FLOAT,GL_FALSE,stride,(GLsizei)offsetof(Vertex, normal)});
    obj->vertexAttributes.push_back({2,2,GL_FLOAT,GL_FALSE,stride,(GLsizei)offsetof(Vertex, uv)});
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);

    obj->subMeshes.clear();
    GLSubMesh sm;
    sm.vertexOffset = 0; sm.indexOffset = 0; sm.indexCount = (GLsizei)idx.size();
    sm.materialIndex = 0; sm.primitiveType = GL_TRIANGLES;
    obj->subMeshes.push_back(sm);

    obj->materials.clear();
    GLMaterial mat;
    mat.diffuseMap = 0; mat.normalMap = 0; mat.specularMap = 0; mat.emissiveMap = 0;
    mat.baseColor = glm::vec4(1,1,1,1);
    mat.roughness = 0.5f; mat.metalness = 0.0f; mat.emissiveIntensity = 0.0f; mat.opacity = 1.0f;
    mat.translucent = false;
    obj->materials.push_back(mat);

    obj->hasSkeleton = false;
    obj->skeleton.jointBuffer = 0; obj->skeleton.jointCount = 0; obj->skeleton.inverseBindBuffer = 0;
    obj->animations.clear();
    obj->boundingMin = minPos;
    obj->boundingMax = maxPos;
    obj->instanceBuffer = 0; obj->instanceCount = 0;

    Logger.Info("Sphere generated", NLLogger::GREEN, false, "GLModel");
    return obj;
}
GLModelObject* AssetManager::loadCube(glm::vec3 size) {
    Logger.Info("Generating cube: " + std::to_string(size.x) + "x" + std::to_string(size.y) + "x" + std::to_string(size.z), NLLogger::GREEN, false, "GLModel");
    float hx = size.x/2, hy = size.y/2, hz = size.z/2;
    struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
    auto addFace = [&](glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 norm) {
        int base = (int)verts.size();
        verts.push_back({v0, norm, {0,0}});
        verts.push_back({v1, norm, {1,0}});
        verts.push_back({v2, norm, {1,1}});
        verts.push_back({v3, norm, {0,1}});
        idx.insert(idx.end(), {static_cast<unsigned int>(base+0), static_cast<unsigned int>(base+1), static_cast<unsigned int>(base+2), static_cast<unsigned int>(base+0), static_cast<unsigned int>(base+2), static_cast<unsigned int>(base+3)});
        for (int i = 0; i < 4; ++i) {
            minPos = glm::min(minPos, verts[base+i].pos);
            maxPos = glm::max(maxPos, verts[base+i].pos);
        }
    };
    addFace({-hx,-hy, hz}, { hx,-hy, hz}, { hx, hy, hz}, {-hx, hy, hz}, {0,0,1});
    addFace({ hx,-hy,-hz}, {-hx,-hy,-hz}, {-hx, hy,-hz}, { hx, hy,-hz}, {0,0,-1});
    addFace({-hx,-hy,-hz}, {-hx,-hy, hz}, {-hx, hy, hz}, {-hx, hy,-hz}, {-1,0,0});
    addFace({ hx,-hy, hz}, { hx,-hy,-hz}, { hx, hy,-hz}, { hx, hy, hz}, {1,0,0});
    addFace({-hx,-hy,-hz}, { hx,-hy,-hz}, { hx,-hy, hz}, {-hx,-hy, hz}, {0,-1,0});
    addFace({-hx, hy, hz}, { hx, hy, hz}, { hx, hy,-hz}, {-hx, hy,-hz}, {0,1,0});

    GLModelObject* obj = new GLModelObject();
    glGenVertexArrays(1, &obj->vao);
    glGenBuffers(1, &obj->vbo);
    glGenBuffers(1, &obj->ibo);
    glBindVertexArray(obj->vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    obj->vertexCount = (GLsizei)verts.size();
    obj->indexCount = (GLsizei)idx.size();

    GLsizei stride = sizeof(Vertex);
    obj->vertexAttributes.clear();
    obj->vertexAttributes.push_back({0,3,GL_FLOAT,GL_FALSE,stride,0});
    obj->vertexAttributes.push_back({1,3,GL_FLOAT,GL_FALSE,stride,(GLsizei)offsetof(Vertex, normal)});
    obj->vertexAttributes.push_back({2,2,GL_FLOAT,GL_FALSE,stride,(GLsizei)offsetof(Vertex, uv)});
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);

    obj->subMeshes.clear();
    GLSubMesh sm;
    sm.vertexOffset = 0;
    sm.indexOffset = 0;
    sm.indexCount = (GLsizei)idx.size();
    sm.materialIndex = 0;
    sm.primitiveType = GL_TRIANGLES;
    obj->subMeshes.push_back(sm);

    obj->materials.clear();
    GLMaterial mat;
    mat.diffuseMap = 0; mat.normalMap = 0; mat.specularMap = 0; mat.emissiveMap = 0;
    mat.baseColor = glm::vec4(1,1,1,1);
    mat.roughness = 0.5f; mat.metalness = 0.0f; mat.emissiveIntensity = 0.0f; mat.opacity = 1.0f;
    mat.translucent = false;
    obj->materials.push_back(mat);

    obj->hasSkeleton = false;
    obj->skeleton.jointBuffer = 0; obj->skeleton.jointCount = 0; obj->skeleton.inverseBindBuffer = 0;
    obj->animations.clear();
    obj->boundingMin = minPos;
    obj->boundingMax = maxPos;
    obj->instanceBuffer = 0; obj->instanceCount = 0;

    Logger.Info("Cube generated", NLLogger::GREEN, false, "GLModel");
    return obj;
}
GLModelObject* AssetManager::loadPlane(glm::vec2 size) {
    Logger.Info("Generating plane: " + std::to_string(size.x) + "x" + std::to_string(size.y), NLLogger::GREEN, false, "GLModel");
    float hw = size.x/2, hh = size.y/2;
    struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };
    std::vector<Vertex> verts = {
        {{-hw, 0, -hh}, {0,1,0}, {0,0}},
        {{ hw, 0, -hh}, {0,1,0}, {1,0}},
        {{ hw, 0,  hh}, {0,1,0}, {1,1}},
        {{-hw, 0,  hh}, {0,1,0}, {0,1}}
    };
    std::vector<unsigned int> idx = {0,1,2, 0,2,3};
    glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
    for (const auto& v : verts) { minPos = glm::min(minPos, v.pos); maxPos = glm::max(maxPos, v.pos); }

    GLModelObject* obj = new GLModelObject();
    glGenVertexArrays(1, &obj->vao);
    glGenBuffers(1, &obj->vbo);
    glGenBuffers(1, &obj->ibo);
    glBindVertexArray(obj->vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    obj->vertexCount = (GLsizei)verts.size();
    obj->indexCount = (GLsizei)idx.size();

    GLsizei stride = sizeof(Vertex);
    obj->vertexAttributes.clear();
    obj->vertexAttributes.push_back({0,3,GL_FLOAT,GL_FALSE,stride,0});
    obj->vertexAttributes.push_back({1,3,GL_FLOAT,GL_FALSE,stride,(GLsizei)offsetof(Vertex, normal)});
    obj->vertexAttributes.push_back({2,2,GL_FLOAT,GL_FALSE,stride,(GLsizei)offsetof(Vertex, uv)});
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);

    obj->subMeshes.clear();
    GLSubMesh sm;
    sm.vertexOffset = 0; sm.indexOffset = 0; sm.indexCount = (GLsizei)idx.size();
    sm.materialIndex = 0; sm.primitiveType = GL_TRIANGLES;
    obj->subMeshes.push_back(sm);

    obj->materials.clear();
    GLMaterial mat;
    mat.diffuseMap = 0; mat.normalMap = 0; mat.specularMap = 0; mat.emissiveMap = 0;
    mat.baseColor = glm::vec4(1,1,1,1);
    mat.roughness = 0.5f; mat.metalness = 0.0f; mat.emissiveIntensity = 0.0f; mat.opacity = 1.0f;
    mat.translucent = false;
    obj->materials.push_back(mat);

    obj->hasSkeleton = false;
    obj->skeleton.jointBuffer = 0; obj->skeleton.jointCount = 0; obj->skeleton.inverseBindBuffer = 0;
    obj->animations.clear();
    obj->boundingMin = minPos;
    obj->boundingMax = maxPos;
    obj->instanceBuffer = 0; obj->instanceCount = 0;

    Logger.Info("Plane generated", NLLogger::GREEN, false, "GLModel");
    return obj;
}
GLModelObject* AssetManager::loadCapsule(float radius, float height) {
    Logger.Info("Generating capsule, radius: " + std::to_string(radius) + ", height: " + std::to_string(height), NLLogger::GREEN, false, "GLModel");
    int seg = 32;
    float halfH = height / 2.0f;
    struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
    auto addHemisphere = [&](float yOff, float sign) {
        for (int j = 0; j <= seg/2; ++j) {
            float theta = (float)j / (seg/2) * 3.14159265f / 2.0f;
            float sinT = sin(theta), cosT = cos(theta);
            float y = yOff + sign * radius * cosT;
            for (int i = 0; i <= seg; ++i) {
                float phi = (float)i / seg * 2.0f * 3.14159265f;
                float sinP = sin(phi), cosP = cos(phi);
                glm::vec3 p = radius * glm::vec3(sinT*cosP, sign*cosT, sinT*sinP);
                p.y += yOff;
                verts.push_back({p, glm::normalize(p - glm::vec3(0,yOff,0)), {(float)i/seg, (float)j/(seg/2)}});
                minPos = glm::min(minPos, p); maxPos = glm::max(maxPos, p);
            }
        }
    };
    addHemisphere(halfH, 1.0f);
    addHemisphere(-halfH, -1.0f);
    for (int j = 0; j <= seg/2; ++j) {
        float y = -halfH + (float)j / (seg/2) * height;
        float t = (float)j / (seg/2);
        for (int i = 0; i <= seg; ++i) {
            float phi = (float)i / seg * 2.0f * 3.14159265f;
            float sinP = sin(phi), cosP = cos(phi);
            glm::vec3 p = radius * glm::vec3(cosP, 0, sinP);
            p.y = y;
            verts.push_back({p, glm::normalize(glm::vec3(cosP, 0, sinP)), {(float)i/seg, t}});
            minPos = glm::min(minPos, p); maxPos = glm::max(maxPos, p);
        }
    }
    int cylBase = (seg/2 + 1);
    for (int j = 0; j < seg/2; ++j) {
        for (int i = 0; i < seg; ++i) {
            int a = j*(seg+1)+i, b = a+1, c = (j+1)*(seg+1)+i, d = c+1;
            idx.insert(idx.end(), {static_cast<unsigned int>(a), static_cast<unsigned int>(c), static_cast<unsigned int>(b), static_cast<unsigned int>(b), static_cast<unsigned int>(c), static_cast<unsigned int>(d)});
        }
    }
    for (int j = 0; j < seg/2; ++j) {
        for (int i = 0; i < seg; ++i) {
            int a = cylBase + j*(seg+1)+i, b = a+1, c = cylBase + (j+1)*(seg+1)+i, d = c+1;
            idx.insert(idx.end(), {static_cast<unsigned int>(a), static_cast<unsigned int>(c), static_cast<unsigned int>(b), static_cast<unsigned int>(b), static_cast<unsigned int>(c), static_cast<unsigned int>(d)});
        }
    }
    int bottomBase = cylBase + (seg/2)*(seg+1);
    for (int j = 0; j < seg/2; ++j) {
        for (int i = 0; i < seg; ++i) {
            int a = bottomBase + j*(seg+1)+i, b = a+1, c = bottomBase + (j+1)*(seg+1)+i, d = c+1;
            idx.insert(idx.end(), {static_cast<unsigned int>(a), static_cast<unsigned int>(c), static_cast<unsigned int>(b), static_cast<unsigned int>(b), static_cast<unsigned int>(c), static_cast<unsigned int>(d)});
        }
    }
    GLModelObject* obj = new GLModelObject();
    glGenVertexArrays(1, &obj->vao);
    glGenBuffers(1, &obj->vbo);
    glGenBuffers(1, &obj->ibo);
    glBindVertexArray(obj->vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    obj->vertexCount = (GLsizei)verts.size();
    obj->indexCount = (GLsizei)idx.size();

    GLsizei stride = sizeof(Vertex);
    obj->vertexAttributes.clear();
    obj->vertexAttributes.push_back({0,3,GL_FLOAT,GL_FALSE,stride,0});
    obj->vertexAttributes.push_back({1,3,GL_FLOAT,GL_FALSE,stride,(GLsizei)offsetof(Vertex, normal)});
    obj->vertexAttributes.push_back({2,2,GL_FLOAT,GL_FALSE,stride,(GLsizei)offsetof(Vertex, uv)});
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);

    obj->subMeshes.clear();
    GLSubMesh sm;
    sm.vertexOffset = 0; sm.indexOffset = 0; sm.indexCount = (GLsizei)idx.size();
    sm.materialIndex = 0; sm.primitiveType = GL_TRIANGLES;
    obj->subMeshes.push_back(sm);

    obj->materials.clear();
    GLMaterial mat;
    mat.diffuseMap = 0; mat.normalMap = 0; mat.specularMap = 0; mat.emissiveMap = 0;
    mat.baseColor = glm::vec4(1,1,1,1);
    mat.roughness = 0.5f; mat.metalness = 0.0f; mat.emissiveIntensity = 0.0f; mat.opacity = 1.0f;
    mat.translucent = false;
    obj->materials.push_back(mat);

    obj->hasSkeleton = false;
    obj->skeleton.jointBuffer = 0; obj->skeleton.jointCount = 0; obj->skeleton.inverseBindBuffer = 0;
    obj->animations.clear();
    obj->boundingMin = minPos;
    obj->boundingMax = maxPos;
    obj->instanceBuffer = 0; obj->instanceCount = 0;

    Logger.Info("Capsule generated", NLLogger::GREEN, false, "GLModel");
    return obj;
}