#include "app_config.h"
#include <fstream>
#include <sstream>

void AppConfig::Load(const fs::path& storeFile) {
    storePath_ = storeFile;
    lastRootPath_.clear();

    std::ifstream f(storePath_, std::ios::binary);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line)) {
        size_t tab = line.find('\t');
        if (tab == std::string::npos) continue;
        std::string key = line.substr(0, tab);
        std::string value = line.substr(tab + 1);
        if (key == "lastRootPath") lastRootPath_ = value;
    }
}

void AppConfig::Save() const {
    std::ofstream f(storePath_, std::ios::binary | std::ios::trunc);
    if (!f.is_open()) return;
    f << "lastRootPath\t" << lastRootPath_ << "\n";
}

void AppConfig::SetLastRootPath(const std::string& utf8Path) {
    lastRootPath_ = utf8Path;
    Save();
}
