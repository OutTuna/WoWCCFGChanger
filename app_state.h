#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class MetaStore;

enum class Faction {
    Unknown = 0,
    Alliance,
    Horde
};

inline const char* FactionName(Faction f) {
    switch (f) {
        case Faction::Alliance: return "Alliance";
        case Faction::Horde:    return "Horde";
        default:                return "Unknown";
    }
}

struct CharacterInfo {
    std::string name;
    std::string realm;
    std::string account;
    fs::path    path;
    Faction     faction = Faction::Unknown;
    bool        factionDetected = false;

    std::string race;
    std::string wowClass;
    bool        metaIsManual = false;
};

struct AccountInfo {
    std::string name;
    fs::path    path;
    bool        hasAccountWideConfig = false;
    std::vector<CharacterInfo> characters;
};

struct WowInstall {
    fs::path root;
    fs::path wtfPath;
    std::vector<AccountInfo> accounts;
};

struct CopyOptions {
    bool copyClientConfig   = true;
    bool copyMacros         = true;
    bool copyKeybinds       = true;
    bool copyLayout         = true;
    bool copyAddonSaved     = true;
    bool copyAccountWideSV  = false;
    bool overwriteExisting  = true;
};

struct AppState {
    WowInstall install;

    int selectedSrcAccount = -1;
    int selectedSrcChar    = -1;

    int selectedDstAccount = -1;
    int selectedDstChar    = -1;

    CopyOptions options;

    std::vector<std::string> log;
    bool scanned = false;

    MetaStore* metaStore = nullptr;
};
