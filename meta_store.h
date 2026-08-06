#pragma once
#include "app_state.h"
#include <unordered_map>
#include <optional>

struct CharMeta {
    Faction     faction = Faction::Unknown;
    std::string race;
    std::string wowClass;
};

class MetaStore {
public:

    void Load(const fs::path& storeFile);

    void Save() const;

    std::optional<CharMeta> Get(const std::string& account,
                                 const std::string& realm,
                                 const std::string& character) const;

    void Set(const std::string& account,
             const std::string& realm,
             const std::string& character,
             const CharMeta& meta);


    void ApplyOverrides(WowInstall& install) const;

private:
    fs::path storePath_;
    std::unordered_map<std::string, CharMeta> entries_;

    static std::string MakeKey(const std::string& account,
                                const std::string& realm,
                                const std::string& character);
};
