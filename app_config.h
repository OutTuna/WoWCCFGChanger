#pragma once
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class AppConfig {
public:
    void Load(const fs::path& storeFile);
    void Save() const;

    std::string GetLastRootPath() const { return lastRootPath_; }
    void SetLastRootPath(const std::string& utf8Path);

private:
    fs::path storePath_;
    std::string lastRootPath_;
};
