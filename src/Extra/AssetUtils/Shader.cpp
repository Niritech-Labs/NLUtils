#include "../../../include/NLUtils/Extra/AssetsUtils.hpp"
#include <glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <format>
#include <string>
#include <algorithm>
#include <filesystem>
#include <cmath>
#include <unordered_map>

inline std::string AssetManager::preprocessShaderFile(const std::string& filepath, const std::string& baseDir, std::unordered_map<std::string, std::string>& cache) {
		auto it = cache.find(filepath);
		if (it != cache.end()) return it->second;
		std::ifstream file(filepath);
		if (!file.is_open()) { 
			Logger.Error("Cannot open:"+ filepath,false,"shaders");
			return ""; 
		}
		std::stringstream ss;
		std::string line;
		while (std::getline(file, line)) {
			size_t pos = line.find("#include");
			if (pos != std::string::npos) {
				size_t start = line.find('"', pos);
				size_t end = line.find('"', start + 1);
				if (start != std::string::npos && end != std::string::npos) {
					std::string inc = line.substr(start + 1, end - start - 1);
					std::string incPath;
					if (inc[0] == '/') incPath = baseDir + inc;
					else {
						size_t lastSlash = filepath.find_last_of("/\\");
						std::string curDir = (lastSlash != std::string::npos) ? filepath.substr(0, lastSlash + 1) : "";
						incPath = curDir + inc;
					}
					std::string incContent = preprocessShaderFile(incPath, baseDir, cache);
					ss << "// #include " << inc << "\n" << incContent << "\n";
					continue;
				}
			}
			ss << line << "\n";
		}
		cache[filepath] = ss.str();
		return ss.str();
}

inline GLuint AssetManager::compileComplexShader(GLenum type, const std::string& filepath, const std::string& baseDir,const std::string& version) {
		std::unordered_map<std::string, std::string> cache;
		std::string source = preprocessShaderFile(filepath, baseDir, cache);
		if (source.empty()) return 0;
        
		std::string prefix = "#version " + version + "\n";
		if (type == GL_VERTEX_SHADER)
			prefix += "#define VSH\n";
		else if (type == GL_FRAGMENT_SHADER)
			prefix += "#define FSH\n";

		std::string finalSource = prefix + source;

		GLuint shader = glCreateShader(type);
		const char* src = finalSource.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char log[512];
			glGetShaderInfoLog(shader, 512, nullptr, log);
			Logger.Error("["+filepath+"] Shader compile error: "+log,false,"shaders");
			return 0;
		}
		return shader;
}
inline GLuint AssetManager::createPipelineProgram(const std::string& shaderPath, const std::string& baseDir,const std::string& version) {
		GLuint vs = compileComplexShader(GL_VERTEX_SHADER, shaderPath, baseDir, version);
		GLuint fs = compileComplexShader(GL_FRAGMENT_SHADER, shaderPath, baseDir, version);
		if (!vs || !fs) return 0;
		GLuint prog = glCreateProgram();
		glAttachShader(prog, vs);
		glAttachShader(prog, fs);
		glLinkProgram(prog);
		int success;
		glGetProgramiv(prog, GL_LINK_STATUS, &success);
		if (!success) {
			char log[512];
			glGetProgramInfoLog(prog, 512, nullptr, log);
			Logger.Error(std::format("Link error: ", log),false, "shaders");
		
		}
		glDeleteShader(vs);
		glDeleteShader(fs);
		return prog;
}

GLuint AssetManager::LoadComplexShader(const std::string& shaderPath, const std::string& baseDir,const std::string& shaderVersion) {
	Logger.Info("Loading complex shader: " + shaderPath + ", with version: " + shaderVersion,NLLogger::GREEN,false,"shaders");
	if (!GLInit) {
        Logger.Warning("GLAD is not initialized");
        return 0;
    }
	GLuint prog = createPipelineProgram(shaderPath, baseDir,shaderVersion);
	if (prog) shaders.push_back(prog);
	return prog;
}

GLuint AssetManager::LoadComplexShaderFromString(std::string shader,const std::string& shaderVersion) {
	Logger.Info("Loading complex shader: from string, with version: " + shaderVersion,NLLogger::GREEN,false,"shaders");
	if (!GLInit) {
        Logger.Warning("GLAD is not initialized","shaders");
        return 0;
    }
	std::filesystem::path temp = "temp.glsl";
	std::ofstream file(temp);
	GLuint prog = 0;
	if (file.is_open()) {
        file << shader << std::endl;
        file.close();
		prog = createPipelineProgram("temp.glsl", "./",shaderVersion);
    } else {
		Logger.Warning("Cannot create tempfile","shaders");
    }

	try {std::filesystem::remove(temp);} 
	catch (...) {}

	
	if (prog) shaders.push_back(prog);
	return prog;
}

