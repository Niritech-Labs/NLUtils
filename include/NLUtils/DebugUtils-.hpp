#include <functional>
#include <string>
#include <format>
#include <vector>
#include <queue>
#include <thread>
#include <unordered_map>
#include "AtomicUtils.hpp"
#include "Logger.hpp"
#include "TextUtils.hpp"
#include <replxx.hxx>

class NLInspector {
public:
    void Setup(bool production);
    void SetBlacklist(std::vector<std::string> bl);
    void SetWhitelist(std::vector<std::string> wl);
    void RegisterObject(void* obj,std::function<std::string()> repres__func,std::string name);
    void DeleteObject(std::string name);
    void Update();
private:
    NLLogger Logger;
    NLAtomic<std::unordered_map<std::string,std::pair<std::function<std::string()>,void*>>> objects;
    replxx::Replxx ReplXX;
    NLAtomic<std::queue<std::string>> commands;
    void __on_critical_exit();
    static void ____on_critical_exit____(NLInspector* instance);
};

