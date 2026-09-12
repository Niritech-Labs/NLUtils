#include "../../../../include/NLUtils/Extra/AssetsUtils.hpp"
#include <sstream>
#include <functional>

AnimationChannel COLLADALoader::parseAnimationChannel(const pugi::xml_node& channelNode, const std::unordered_map<std::string, pugi::xml_node>& sourceMap, const std::unordered_map<std::string, pugi::xml_node>& samplerMap) {
    AnimationChannel ch;
    ch.target = channelNode.attribute("target").as_string();

    std::string samplerRef = channelNode.attribute("source").as_string();
    if (!samplerRef.empty() && samplerRef[0] == '#') samplerRef.erase(0, 1);
    auto samplerIt = samplerMap.find(samplerRef);
    if (samplerIt == samplerMap.end()) return ch;

    auto samplerNode = samplerIt->second;
    for (auto input : samplerNode.children("input")) {
        std::string semantic = input.attribute("semantic").as_string();
        std::string src = input.attribute("source").as_string();
        if (!src.empty() && src[0] == '#') src.erase(0, 1);
        auto srcNodeIt = sourceMap.find(src);
        if (srcNodeIt == sourceMap.end()) continue;
        auto srcNode = srcNodeIt->second;
        auto floatArray = srcNode.child("float_array");
        if (floatArray) {
            std::stringstream ss(floatArray.text().as_string());
            std::vector<float> vals;
            float v;
            while (ss >> v) vals.push_back(v);

            if (semantic == "INPUT") ch.times = vals;
            else if (semantic == "OUTPUT") ch.values = vals;
            else if (semantic == "IN_TANGENT") ch.inTangents = vals;
            else if (semantic == "OUT_TANGENT") ch.outTangents = vals;
        } else if (semantic == "INTERPOLATION") {
            auto nameArray = srcNode.child("Name_array");
            if (nameArray) {
                std::stringstream ns(nameArray.text().as_string());
                std::string interp;
                while (ns >> interp) ch.interpolation.push_back(interp);
            }
        }
    }
    return ch;
}

std::unordered_map<std::string, AnimationClip> COLLADALoader::parseAnimations(const pugi::xml_node& libAnimNode) {
    std::unordered_map<std::string, AnimationClip> clips;

    std::function<void(const pugi::xml_node&)> processAnim = [&](const pugi::xml_node& animNode) {
        AnimationClip clip;
        clip.name = animNode.attribute("id").as_string();

        std::unordered_map<std::string, pugi::xml_node> sourceMap;
        std::unordered_map<std::string, pugi::xml_node> samplerMap;
        for (auto src : animNode.children("source")) {
            std::string id = src.attribute("id").as_string();
            if (!id.empty()) sourceMap[id] = src;
        }
        for (auto samp : animNode.children("sampler")) {
            std::string id = samp.attribute("id").as_string();
            if (!id.empty()) samplerMap[id] = samp;
        }

        for (auto chNode : animNode.children("channel")) {
            AnimationChannel ch = parseAnimationChannel(chNode, sourceMap, samplerMap);
            if (!ch.target.empty() && !ch.values.empty()) {
                clip.channels.push_back(std::move(ch));
            }
        }

        if (!clip.channels.empty()) {
            for (auto& ch : clip.channels) {
                if (!ch.times.empty()) {
                    if (ch.times.front() < clip.startTime || clip.startTime == 0.0f)
                        clip.startTime = ch.times.front();
                    if (ch.times.back() > clip.endTime)
                        clip.endTime = ch.times.back();
                }
            }
            clip.duration = clip.endTime - clip.startTime;
            clips[clip.name] = std::move(clip);
        }

        for (auto childAnim : animNode.children("animation")) {
            processAnim(childAnim);
        }
    };

    for (auto anim : libAnimNode.children("animation")) {
        processAnim(anim);
    }
    return clips;
}