#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad.h> 

struct Skeleton {
    std::vector<std::string> jointNames;
    std::vector<int> parentIndices;
    std::vector<glm::mat4> localBindPoses;
    std::vector<glm::mat4> inverseBindPoses;
};

struct VertexAttribute {
    enum Type {
        POSITION,
        NORMAL,
        TEXCOORD,
        TANGENT,
        BITANGENT,
        JOINT_INDICES,
        JOINT_WEIGHTS,
        COLOR,
        CUSTOM
    };
    Type type;
    int components;
    int offset;
    int stride;
};

struct VertexBuffer {
    std::vector<float> data;
    std::vector<VertexAttribute> attributes;
    int vertexCount = 0;
    int vertexSize = 0;
};

struct IndexBuffer {
    std::vector<unsigned int> data;
    int count = 0;
};

struct SubMesh {
    std::string materialName;
    int vertexStart = 0;
    int vertexCount = 0;
    int indexStart = 0;
    int indexCount = 0;
    int rawIndexStart = 0;
    int rawIndexCount = 0;
};

struct AnimationChannel {
    std::string target;
    std::vector<float> times;
    std::vector<float> values;
    std::vector<std::string> interpolation;
    std::vector<float> inTangents;
    std::vector<float> outTangents;
};

struct AnimationClip {
    std::string name;
    float duration = 0.0f;
    float startTime = 0.0f;
    float endTime = 0.0f;
    std::vector<AnimationChannel> channels;
};

struct Material {
    std::string name;
    std::string diffuseMap;
    std::string normalMap;
    std::string specularMap;
    std::string emissiveMap;
    glm::vec3 ambient = glm::vec3(0.2f);
    glm::vec3 diffuse = glm::vec3(0.8f);
    glm::vec3 specular = glm::vec3(0.5f);
    float shininess = 32.0f;
    int type = 0;
    float metalness = 0.0f;
    float roughness = 0.5f;
    float emission = 0.0f;
    float subsurface = 0.0f;
    float opacity = 1.0f;
};

struct Model {
    std::string name;
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    std::vector<SubMesh> subMeshes;
    Skeleton skeleton;
    std::unordered_map<std::string, AnimationClip> animations;
    std::unordered_map<std::string, Material> materials;
};


struct GLVertexAttribute {
    GLuint location;      
    GLint size;          
    GLenum type;         
    GLboolean normalized;
    GLsizei stride;       
    GLsizei offset;      
};


struct GLMaterial {
    // Текстуры
    GLuint diffuseMap = 0;
    GLuint normalMap = 0;
    GLuint specularMap = 0;
    GLuint emissiveMap = 0;
    glm::vec4 baseColor = glm::vec4(1.0f);
    float roughness = 0.5f;
    float metalness = 0.0f;
    float emissiveIntensity = 0.0f;
    float opacity = 1.0f;
    bool translucent = false;
    GLuint materialUBO = 0; 
};

struct GLSubMesh {
    GLuint vertexOffset;  
    GLuint indexOffset; 
    GLuint indexCount;    
    GLuint materialIndex;  
    GLenum primitiveType = GL_TRIANGLES; 
};

// Скелет – для скиннинга
struct GLSkeleton {
    GLuint jointBuffer = 0;        
    GLuint jointCount = 0;
    GLuint inverseBindBuffer = 0; 
};


struct GLModelObject {
    // Идентификаторы буферов
    GLuint vao = 0;
    GLuint vbo = 0;           
    GLuint ibo = 0;           
    GLuint vertexCount = 0;
    GLuint indexCount = 0;

    std::vector<GLVertexAttribute> vertexAttributes;
    std::vector<GLSubMesh> subMeshes;
    std::vector<GLMaterial> materials;
    GLSkeleton skeleton;
    bool hasSkeleton = false;
    GLuint instanceBuffer = 0;
    GLuint instanceCount = 0;

    glm::vec3 boundingMin;
    glm::vec3 boundingMax;

    std::unordered_map<std::string, AnimationClip> animations;    
    std::vector<glm::mat4> jointMatrices;
    
};