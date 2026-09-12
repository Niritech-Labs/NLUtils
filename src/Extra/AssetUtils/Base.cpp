#include "../../../include/NLUtils/Extra/AssetsUtils.hpp"
void AssetManager::Shutdown() {
    for (GLuint s : shaders) glDeleteProgram(s);
    for (GLuint t : textures) glDeleteTextures(1, &t);
    shaders.clear();
    textureCache.clear();
    textures.clear();
    for (GLModelObject* obj : GLModels) delete obj;
    GLModels.clear();
    GLModelCache.clear();

    for (Model* m : models) delete m;
    models.clear();
    modelCache.clear();
}

void AssetManager::SetBlacklist(std::vector<std::string> bl) {
    this->Logger.SetBlacklist(bl);
}
void AssetManager::SetWhitelist(std::vector<std::string> wl) {
    this->Logger.SetWhitelist(wl);
}

void AssetManager::Setup(bool production) {
    Logger.Setup(production,"AssetManager");
    ColladaLoader.Setup(&Logger,production);
}
void AssetManager::Setup(bool production,GLADloadproc proc) {
    Logger.Setup(production,"AssetManager");
    ColladaLoader.Setup(&Logger,production);
    GLInit = true;
    if (!gladLoadGLLoader(proc)) { 
        Logger.Error("GL GLAD init failed",false);
        GLInit = false;
    }
}


AssetManager::AssetManager() {}
AssetManager::AssetManager(bool pproduction,GLADloadproc proc) {
    this->Setup(pproduction,proc);
}