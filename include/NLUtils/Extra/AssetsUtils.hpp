#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../XMLUtils.hpp"
#include "GMA.hpp"
#include <Nuklear.h>
#include "../Logger.hpp"

class COLLADALoader {
public:
    void Setup(NLLogger* logger, bool production);
    Model LoadModel(const std::string path);

private:
    NLLogger* Logger;
    XMLManager xmlm;

    struct ParsedSource;
    struct VertexInputBinding;
    struct TempVertex;
    struct VertexKey;
    struct VertexKeyHash;
    struct AttributeIndices;

    ParsedSource parseSource(const pugi::xml_node& sourceNode);
    glm::vec3 getVec3(const ParsedSource& src, int idx);
    glm::vec2 getVec2(const ParsedSource& src, int idx);

    bool parseStaticGeometry(const pugi::xml_node& meshNode, std::vector<TempVertex>& outVertices, std::vector<int>& outRawIndices, std::vector<SubMesh>& outSubMeshes, std::unordered_map<std::string, int>& materialIndexMap);
    bool parseSkinning(const pugi::xml_node& skinNode, Skeleton& outSkeleton, std::vector<TempVertex>& outVertices);
    bool parseSkeletonHierarchy(const pugi::xml_node& visualSceneNode, const std::vector<std::string>& jointNames, std::vector<int>& parentIndices, std::vector<glm::mat4>& localBindPoses);

    AnimationChannel parseAnimationChannel(const pugi::xml_node& channelNode, const std::unordered_map<std::string, pugi::xml_node>& sourceMap, const std::unordered_map<std::string, pugi::xml_node>& samplerMap);
    std::unordered_map<std::string, AnimationClip> parseAnimations(const pugi::xml_node& libAnimNode);

    Material parseMaterial(const pugi::xml_node& materialNode);
};

struct COLLADALoader::ParsedSource {
    std::vector<float> data;
    int stride = 1;
    int count = 0;
};

struct COLLADALoader::VertexInputBinding {
    int offset;
    std::string semantic;
    std::string sourceId;
};

struct COLLADALoader::TempVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::ivec4 jointIndices = glm::ivec4(0);
    glm::vec4 jointWeights = glm::vec4(0.0f);
};

struct COLLADALoader::AttributeIndices {
    int posIdx;
    int normIdx;
    int texIdx;
};

struct COLLADALoader::VertexKey {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::ivec4 jointIndices;
    glm::vec4 jointWeights;
    bool operator==(const VertexKey& other) const;
};

struct COLLADALoader::VertexKeyHash {
    size_t operator()(const VertexKey& k) const;
};



class AssetManager {  
public:
    AssetManager();
    AssetManager(bool production,GLADloadproc proc);
    struct ModelType {
        enum Type {PLANE,CUBE,SPHERE,CAPSULE,COLLADA,DYNAMIC} type = PLANE;
        struct Data {
            std::string ColladaPath;
            glm::vec2 CapsuleSize;
            glm::vec3 CubeSize;
            glm::vec1 SphereRadius;
            glm::vec2 PlaneSize;
            uint dynamicId;
        } data;
        uint GetHash() {
            std::hash<std::string> strHasher;
            std::hash<float> floatHasher;
            uint h = 0;
            switch (type) {
                case PLANE: {
                    h = floatHasher(data.PlaneSize.x);
                    h ^= floatHasher(data.PlaneSize.y) << 1;
                    break;
                }
                case CUBE: {
                    h = floatHasher(data.CubeSize.x);
                    h ^= floatHasher(data.CubeSize.y) << 1;
                    h ^= floatHasher(data.CubeSize.z) << 2;
                    break;
                }
                case SPHERE: {
                    h = floatHasher(data.SphereRadius.x);
                    break;
                }
                case CAPSULE: {
                    h = floatHasher(data.CapsuleSize.x);
                    h ^= floatHasher(data.CapsuleSize.y) << 1;
                    break;
                }
                case COLLADA: {
                    h = (uint)strHasher(data.ColladaPath);
                    break;
                }
                case DYNAMIC: {
                    h = data.dynamicId;
                    break;
                }
                default: {
                    h = 0;
                    break;
                }
            }
            h ^= (uint)type << 24;
            return h;
        }
    };
    ~AssetManager() {
        Shutdown();
    }
    void Setup(bool production);
    void Setup(bool production,GLADloadproc proc);
    GLuint LoadComplexShaderFromString(std::string shader,const std::string& shaderVersion);
    GLuint LoadComplexShader(const std::string& shaderPath, const std::string& baseDir,const std::string& shaderVersion);
    GLModelObject* GetGLMO(ModelType type);
    Model* GetModel(ModelType type);
    GLuint LoadTexture(const std::string& path);
    struct nk_image LoadImage(const std::string& path);
    struct nk_nine_slice LoadImageNineSlice(const std::string& path, int left, int top, int right, int bottom);
    void Shutdown();
    void SetBlacklist(std::vector<std::string> bl);
    void SetWhitelist(std::vector<std::string> wl);

private:
    bool GLInit = false;
    COLLADALoader ColladaLoader;

    NLLogger Logger;
    std::vector<GLuint> shaders;
    std::string preprocessShaderFile(const std::string& filepath, const std::string& baseDir, std::unordered_map<std::string, std::string>& cache);
    GLuint compileComplexShader(GLenum type, const std::string& filepath, const std::string& baseDir,const std::string& version);
    GLuint createPipelineProgram(const std::string& shaderPath, const std::string& baseDir,const std::string& version);
    std::unordered_map<std::string, GLuint> textureCache;
    std::vector<GLuint> textures;
    std::map<std::string, std::pair<int, int>> textureSizes;
    GLuint loadTexture(const std::string& path);
    GLuint loadUITexture(const std::string& path);
    std::unordered_map<uint, GLModelObject*> GLModelCache;
    std::vector<GLModelObject*> GLModels;
    GLModelObject* loadCollada(std::string path);
    GLModelObject* loadCapsule(float radius,float heihgt);
    GLModelObject* loadSphere(float radius);
    GLModelObject* loadCube(glm::vec3 size);
    GLModelObject* loadPlane(glm::vec2 size);
    GLModelObject* loadDynamicGLMO();
    

    std::unordered_map<uint, Model*> modelCache;
    std::vector<Model*> models;
    Model* loadColladaBase(std::string path);
    Model* loadCapsuleBase(float radius,float heihgt);
    Model* loadSphereBase(float radius);
    Model* loadCubeBase(glm::vec3 size);
    Model* loadPlaneBase(glm::vec2 size);
};
