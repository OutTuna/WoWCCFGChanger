#include "meta_store.h"
#include "utf8_utils.h"
#include <fstream>
#include <sstream>


std::string MetaStore::MakeKey(const std::string& account,
                                const std::string& realm,
                                const std::string& character) {
    return account + '\x1F' + realm + '\x1F' + character;
}

void MetaStore::Load(const fs::path& storeFile) {
    storePath_ = storeFile;
    entries_.clear();

    std::ifstream f(storePath_, std::ios::binary);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;

        size_t tab1 = line.find('\t');
        if (tab1 == std::string::npos) continue;
        std::string key = line.substr(0, tab1);

        size_t tab2 = line.find('\t', tab1 + 1);
        size_t tab3 = (tab2 == std::string::npos) ? std::string::npos : line.find('\t', tab2 + 1);
        if (tab2 == std::string::npos || tab3 == std::string::npos) continue;

        std::string factionStr = line.substr(tab1 + 1, tab2 - tab1 - 1);
        std::string race       = line.substr(tab2 + 1, tab3 - tab2 - 1);
        std::string wowClass   = line.substr(tab3 + 1);

        CharMeta meta;
        meta.race     = race;
        meta.wowClass = wowClass;
        meta.faction  = (factionStr == "Horde") ? Faction::Horde
                       : (factionStr == "Alliance") ? Faction::Alliance
                       : Faction::Unknown;

        entries_[key] = meta;
    }
}

void MetaStore::Save() const {
    std::ofstream f(storePath_, std::ios::binary | std::ios::trunc);
    if (!f.is_open()) return;

    for (auto& [key, meta] : entries_) {
        f << key << '\t' << FactionName(meta.faction) << '\t'
          << meta.race << '\t' << meta.wowClass << '\n';
    }
}

std::optional<CharMeta> MetaStore::Get(const std::string& account,
                                        const std::string& realm,
                                        const std::string& character) const {
    auto it = entries_.find(MakeKey(account, realm, character));
    if (it == entries_.end()) return std::nullopt;
    return it->second;
}

void MetaStore::Set(const std::string& account,
                     const std::string& realm,
                     const std::string& character,
                     const CharMeta& meta) {
    entries_[MakeKey(account, realm, character)] = meta;
    Save();
}

void MetaStore::ApplyOverrides(WowInstall& install) const {
    for (auto& acc : install.accounts) {
        for (auto& ch : acc.characters) {
            auto meta = Get(ch.account, ch.realm, ch.name);
            if (!meta) continue;
            ch.faction        = meta->faction;
            ch.race           = meta->race;
            ch.wowClass       = meta->wowClass;
            ch.factionDetected = (meta->faction != Faction::Unknown);
            ch.metaIsManual   = true;
        }
    }
}
