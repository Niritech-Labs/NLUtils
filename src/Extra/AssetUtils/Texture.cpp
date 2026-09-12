#include "../../../include/NLUtils/Extra/AssetsUtils.hpp"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct nk_image AssetManager::LoadImage(const std::string& path) {
    Logger.Info("Loading nuklear image: " + path,NLLogger::GREEN,false,"textures-nuklear");
    if (!GLInit) {
        Logger.Warning("GLAD is not initialized");
        struct nk_image image;
        return image;
    }
    auto it = textureCache.find(path);
    unsigned int tex = 0;
    
    if (it != textureCache.end()) {tex = it->second;} 
    else {tex = loadUITexture(path);}

    auto size = textureSizes[path];
    struct nk_image img;
    img.handle = nk_handle_id((int)tex);
    img.w = (nk_ushort)size.first;   
    img.h = (nk_ushort)size.second;
    img.region[0] = 0; img.region[1] = 0;
    img.region[2] = (nk_ushort)size.first; img.region[3] = (nk_ushort)size.second;  
    return img;
}


struct nk_nine_slice AssetManager::LoadImageNineSlice(const std::string& path, int left, int top, int right, int bottom) {
    Logger.Info("Loading image nine slice: " + path,NLLogger::GREEN,false,"textures-nuklear");
    if (!GLInit) {
        Logger.Warning("GLAD is not initialized");
        struct nk_nine_slice image;
        return image;
    }
    auto it = textureCache.find(path);
    unsigned int tex = 0;
    
    if (it != textureCache.end()) {tex = it->second;} 
    else {tex = loadUITexture(path);}

    auto size = textureSizes[path];

    struct nk_nine_slice slc;
    slc.img.handle = nk_handle_id((int)tex);
    slc.img.w = (nk_ushort)size.first;   
    slc.img.h = (nk_ushort)size.second;  
    slc.img.region[0] = 0; slc.img.region[1] = 0;
    slc.img.region[2] = (nk_ushort)size.first; slc.img.region[3] = (nk_ushort)size.second;
    
    slc.l = (nk_ushort)left;
    slc.r = (nk_ushort)right;
    slc.t = (nk_ushort)top;
    slc.b = (nk_ushort)bottom;

    return slc;
}

GLuint AssetManager::loadUITexture(const std::string& path) {
    int width, height, channels;
    
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4); 
    if (!data) {
        Logger.Warning("Failed to load UI texture: " + path,"textures-nuklear");
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    
    textureCache[path] = texture;
    textureSizes[path] = {width, height};
    textures.push_back(texture);
    return texture;
}


GLuint AssetManager::loadTexture(const std::string& path) {
        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data) {
            Logger.Warning("Failed to load texture: " + path,"textures-opengl");
            return 0;
        }
        GLenum format, internalFormat;
        switch (channels) {
            case 1: internalFormat = GL_RED;  format = GL_RED;  break;
            case 2: internalFormat = GL_RG;   format = GL_RG;   break;
            case 3: internalFormat = GL_RGB;  format = GL_RGB;  break;
            case 4: internalFormat = GL_RGBA; format = GL_RGBA; break;
            default: internalFormat = GL_RGB; format = GL_RGB; break;
        }
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        textureCache[path] = texture;
        textures.push_back(texture);
        return texture;
}


GLuint AssetManager::LoadTexture(const std::string& path) {
    Logger.Info("Loading texture: " + path,NLLogger::GREEN,false,"textures-opengl");
    if (!GLInit) {
        Logger.Warning("GLAD is not initialized");
        return 0;
    }
    auto it = textureCache.find(path);
    if (it != textureCache.end()) return it->second;
    return loadTexture(path);
}


