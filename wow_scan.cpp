#include "wow_scan.h"
#include "utf8_utils.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>

namespace WowScan {

void DetectFaction(CharacterInfo& c) {
    fs::path svDir = c.path / "SavedVariables";
    if (!fs::exists(svDir) || !fs::is_directory(svDir)) return;

    static const std::regex reHorde(
        R"(\bfaction\s*=\s*["']?Horde["']?)",
        std::regex::icase);
    static const std::regex reAlliance(
        R"(\bfaction\s*=\s*["']?Alliance["']?)",
        std::regex::icase);
    static const std::regex reHorde2(R"(["']Horde["']\s*,?\s*--\s*faction)", std::regex::icase);
    static const std::regex reAlliance2(R"(["']Alliance["']\s*,?\s*--\s*faction)", std::regex::icase);

    for (auto& entry : fs::directory_iterator(svDir)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".lua") continue;

        std::ifstream f(entry.path());
        if (!f.is_open()) continue;

        std::stringstream buf;
        buf << f.rdbuf();
        std::string content = buf.str();

        if (content.size() > 2'000'000) content.resize(2'000'000);

        if (std::regex_search(content, reHorde) || std::regex_search(content, reHorde2)) {
            c.faction = Faction::Horde;
            c.factionDetected = true;
            return;
        }
        if (std::regex_search(content, reAlliance) || std::regex_search(content, reAlliance2)) {
            c.faction = Faction::Alliance;
            c.factionDetected = true;
            return;
        }
    }
    c.faction = Faction::Unknown;
    c.factionDetected = false;
}

bool ScanInstall(WowInstall& install, std::vector<std::string>& log) {
    install.wtfPath = install.root / "WTF";
    install.accounts.clear();

    if (!fs::exists(install.wtfPath) || !fs::is_directory(install.wtfPath)) {
        log.push_back("[!] No WTF folder found at: " + Utf8::PathToUtf8(install.wtfPath));
        return false;
    }

    fs::path accountsRoot = install.wtfPath / "Account";
    if (!fs::exists(accountsRoot)) {
        log.push_back("[!] No WTF/Account folder found.");
        return false;
    }

    for (auto& accEntry : fs::directory_iterator(accountsRoot)) {
        if (!accEntry.is_directory()) continue;

        AccountInfo acc;
        acc.name = Utf8::PathToUtf8(accEntry.path().filename());
        acc.path = accEntry.path();
        acc.hasAccountWideConfig = fs::exists(acc.path / "config-cache.wtf");

        for (auto& realmEntry : fs::directory_iterator(acc.path)) {
            if (!realmEntry.is_directory()) continue;
            std::string realmName = Utf8::PathToUtf8(realmEntry.path().filename());
            if (realmName == "SavedVariables") continue;

            for (auto& charEntry : fs::directory_iterator(realmEntry.path())) {
                if (!charEntry.is_directory()) continue;

                CharacterInfo ch;
                ch.name    = Utf8::PathToUtf8(charEntry.path().filename());
                ch.realm   = realmName;
                ch.account = acc.name;
                ch.path    = charEntry.path();

                DetectFaction(ch);

                acc.characters.push_back(std::move(ch));
            }
        }

        std::sort(acc.characters.begin(), acc.characters.end(),
                   [](const CharacterInfo& a, const CharacterInfo& b) {
                       if (a.realm != b.realm) return a.realm < b.realm;
                       return a.name < b.name;
                   });

        install.accounts.push_back(std::move(acc));
    }

    std::sort(install.accounts.begin(), install.accounts.end(),
               [](const AccountInfo& a, const AccountInfo& b) { return a.name < b.name; });

    log.push_back("[+] Scanned " + std::to_string(install.accounts.size()) + " account(s).");
    return true;
}

}
