#pragma once

#include <string>
#include <filesystem>


namespace NLPath {

class Path {
public:

    Path() {}
    Path(const Path& p) {path = p.path;}
    Path(Path&& p) {path = std::move(p.path);}
    
    Path(const std::string& source) {path = source;}
    Path(std::string&& source) {path = std::move(source);}

#if defined(_WIN32) || defined(_WIN64)
    Path(const std::wstring& source) {path = source;}
    Path(std::wstring&& source) {path = std::move(source);}
    operator std::wstring() const {return path;}
#endif
    Path(const char* source) {path = source;}

    Path& operator=(const Path& p) { path = p.path; return *this; }
    Path& operator=(Path&& p) { path = std::move(p.path); return *this; }
    Path& operator=(const std::string& source) { path = source; return *this; }
    Path& operator=(std::string&& source) { path = std::move(source); return *this; }
    Path& operator=(const char* source) { path = source; return *this; }
    bool operator==(const Path& p) const { return path == p.path; }


    void clear() {path.clear();}
    Path& make_preferred() {path.make_preferred(); return *this;}
    Path& remove_filename() {path.remove_filename(); return *this;}
    Path& replace_filename(const Path& replacement) {path.replace_filename(replacement.path); return *this;}
    Path& replace_extension(const Path& replacement = "") {path.replace_extension(replacement.path); return *this;}
    void swap(Path& other) {std::swap(other.path,path);}


    Path& operator/=(const Path& p) {path /= p.path; return *this;}
    Path operator/(const Path& p) {auto tmp = path / p.path; return Path(tmp);}

    Path& operator+=(const Path& p) {path += p.path; return *this;}
    Path& operator+=(const std::string& str) {path += str; return *this;}
    Path& operator+=(const char* x) {path += x; return *this;}
    
    operator std::string() const {return path.string();}
    std::string string() const {return path.string();}
#if defined(_WIN32) || defined(_WIN64)
    const wchar_t* c_str() const {return path.c_str();}
#else
    const char* c_str() const {return path.c_str();}
#endif
    Path root_name() const {return Path(path.root_name());}
    Path root_directory() const {return Path(path.root_directory());}
    Path root_path() const {return Path(path.root_path());}
    Path relative_path() const {return Path(path.relative_path());}
    Path parent_path() const {return Path(path.parent_path());}
    Path filename() const {return Path(path.filename());}
    Path stem() const {return Path(path.stem());}
    Path extension() const {return Path(path.extension());}

    bool empty() const {return path.empty();}
    bool has_root_name() const {return path.has_root_name();}
    bool has_root_directory() const {return path.has_root_directory();}
    bool has_root_path() const {return path.has_root_path();}
    bool has_relative_path() const {return path.has_relative_path();}
    bool has_parent_path() const {return path.has_parent_path();}
    bool has_filename() const {return path.has_filename();}
    bool has_stem() const {return path.has_stem();}
    bool has_extension() const {return path.has_extension();}
    bool is_absolute() const {return path.is_absolute();}
    bool is_relative() const {return path.is_relative();}

    Path& canonical() {path = std::filesystem::canonical(path); return *this;}
    Path& weakly_canonical() {path = std::filesystem::weakly_canonical(path); return *this;}

    Path& expanduser() {
        std::string original = path.string();
        std::string buf = path.string();
        if (!original.empty() && original[0] == '~') {
            const char* home = nullptr;
#if defined(_WIN32) || defined(_WIN64)
            home = std::getenv("USERPROFILE");
#else
            home = std::getenv("HOME");
#endif
            if (home) {buf.replace(0, 1, home);path = buf;}
        }
        try {weakly_canonical();} 
        catch (...) {path = original;}
        return *this;
    }


    int compare(const Path& p) const {return path.compare(p);}
    int compare(const std::string& str) const {return path.compare(str);}

    std::filesystem::path::iterator begin() const {return path.begin();}
    std::filesystem::path::iterator end() const {return path.end();}

    const std::filesystem::path& getPath() const {return path;}
private:
    std::filesystem::path path;
};
}

namespace std {
    template<>
    struct hash<NLPath::Path> {
        std::size_t operator()(const NLPath::Path& p) const noexcept {
            return std::filesystem::hash_value(p.getPath());
        }
    };
}
