#include "../include/NLUtils/DebugUtils.hpp"


void NLInspector::Setup(bool production) {
    Logger.Setup(production, "Inspector");
    NLInspector::____on_critical_exit____(this);
    ReplXX.set_max_history_size(128);
    ReplXX.set_complete_on_empty(true);
    ReplXX.set_completion_callback([this](std::string const& context, int index) {
        replxx::Replxx::completions_t completions;
        
        if (index >= 4) { 
            auto* objsMap = objects.Get(AccessModifier::R_);
            for (auto const& pair : *objsMap) {
                completions.push_back(pair.first);
            }
            objects.Release(AccessModifier::R_);
        } else {
           
            std::string cmd = "Get ";
            if (cmd.compare(0, context.length(), context) == 0) {
                completions.push_back(cmd);
            }
        }
        
        return completions;
    });

    std::thread([this]() {
        while (true) {
            char const* input = ReplXX.input("inspector-->");
            
            if (!input) {
                auto* queue = commands.Get(AccessModifier::RW);
                queue->push("exit");
                commands.Release(AccessModifier::RW);
                break;
            }

            std::string line(input);
            if (!line.empty()) {
                auto* queue = commands.Get(AccessModifier::RW);
                queue->push(line);
                commands.Release(AccessModifier::RW);

                //if (line == "exit" || line == "quit") {
                //    break;
                //}
            }
        }
    }).detach();
}

void NLInspector::SetBlacklist(std::vector<std::string> bl) {
    Logger.SetBlacklist(bl);
}
void NLInspector::SetWhitelist(std::vector<std::string> wl) {
    Logger.SetWhitelist(wl);
}
void NLInspector::RegisterObject(void* obj,std::function<std::string()> repres__func,std::string name) {
    auto registry = objects.Get(AccessModifier::RW);
    auto it = registry->find(name);
    if (it != registry->end()) {
        Logger.Warning("Object already exists");
    }
    else {
        registry->insert({name,std::make_pair(repres__func,obj)});
    }
    objects.Release(AccessModifier::RW);
}
void NLInspector::DeleteObject(std::string name) {
    auto registry = objects.Get(AccessModifier::RW);
    auto it = registry->find(name);
    if (it != registry->end()) {
        registry->erase(name);
    }
    else {
        Logger.Warning("Object not exists");
    }
    objects.Release(AccessModifier::RW);
}
void NLInspector::Update() {
    std::string command = "";
    std::string Answer = "";

    auto* queue = commands.Get(AccessModifier::RW);
    if (!queue->empty()) {
        command = queue->front();
        queue->pop();
    }
    commands.Release(AccessModifier::RW);

    if (!command.empty()) {
        auto strList = NLTextUtils::Split(command," ",2);
        if (strList[0] == "Get") {
            auto* manager = objects.Get(AccessModifier::R_);
            auto it = manager->find(strList[1]);
            if (it != manager->end()) {
                std::string id = std::format("{:p}", it->second.second);
                Answer += (it->first + "/["+id+"]"+it->second.first() );
                std::cout << "\n";
                Logger.Info(Answer,NLLogger::VIOLET_B,true);
                std::cout << "Inspector-->"<< std::flush;;
            }
            else {
                Logger.Warning("Not found: "+ strList[1]);
            }
            objects.Release(AccessModifier::R_);
        }
        if (strList[0] == "h" || strList[0] == "help") {
            std::cout << "\n";
            Logger.Info("Commands: Get [objectName], h/help",NLLogger::GREEN_B,true);
            std::cout << "\nInspector-->" << std::flush;
        }

    }
}

void NLInspector::____on_critical_exit____(NLInspector* instance) {
    static NLInspector* ctx = instance;
    std::atexit([]() {
        if (ctx) {
            ctx->__on_critical_exit();
        }
    });
    
}

void NLInspector::__on_critical_exit() {
    this->ReplXX.~Replxx();
}