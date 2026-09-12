// COLLADALoader_Materials.cpp
#include "../../../../include/NLUtils/Extra/AssetsUtils.hpp"
#include <sstream>

Material COLLADALoader::parseMaterial(const pugi::xml_node& materialNode) {
    Material mat;
    mat.name = materialNode.attribute("name").as_string();

    auto effectNode = materialNode.child("instance_effect");
    if (!effectNode) return mat;

    std::string effectUrl = effectNode.attribute("url").as_string();
    if (!effectUrl.empty() && effectUrl[0] == '#') effectUrl.erase(0, 1);

    auto doc = materialNode.root();
    auto libEffects = doc.child("COLLADA").child("library_effects");
    if (!libEffects) return mat;

    auto effect = libEffects.find_child_by_attribute("effect", "id", effectUrl.c_str());
    if (!effect) return mat;

    auto profile = effect.child("profile_COMMON");
    if (!profile) return mat;

    auto technique = profile.child("technique");
    if (!technique) return mat;

    auto shader = technique.child("phong");
    if (!shader) shader = technique.child("lambert");
    if (!shader) shader = technique.child("blinn");
    if (!shader) return mat;

    auto diffuse = shader.child("diffuse");
    if (diffuse) {
        auto color = diffuse.child("color");
        if (color) {
            std::stringstream ss(color.text().as_string());
            ss >> mat.diffuse.r >> mat.diffuse.g >> mat.diffuse.b;
        }
        auto texture = diffuse.child("texture");
        if (texture) {
            std::string texSampler = texture.attribute("texture").as_string();
            for (auto newparam : profile.children("newparam")) {
                std::string sid = newparam.attribute("sid").as_string();
                if (sid == texSampler) {
                    auto sampler = newparam.child("sampler2D");
                    if (sampler) {
                        auto source = sampler.child("source");
                        if (source) {
                            std::string surfId = source.text().as_string();
                            for (auto np : profile.children("newparam")) {
                                if (np.attribute("sid").as_string() == surfId) {
                                    auto surface = np.child("surface");
                                    if (surface) {
                                        auto initFrom = surface.child("init_from");
                                        if (initFrom) {
                                            mat.diffuseMap = initFrom.text().as_string();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    auto specular = shader.child("specular");
    if (specular) {
        auto color = specular.child("color");
        if (color) {
            std::stringstream ss(color.text().as_string());
            ss >> mat.specular.r >> mat.specular.g >> mat.specular.b;
        }
    }

    auto shininess = shader.child("shininess");
    if (shininess) {
        auto floatNode = shininess.child("float");
        if (floatNode) {
            mat.shininess = std::stof(floatNode.text().as_string());
        }
    }

    return mat;
}