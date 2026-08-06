#include "ui.h"
#include "wow_scan.h"
#include "app_logic.h"
#include "theme.h"
#include "utf8_utils.h"
#include "meta_store.h"
#include "imgui.h"
#include <cstring>

namespace UI {

static char rootPathBuf[512] = "";

static const char* kAllianceRaces[] = { "Human", "Dwarf", "Night Elf", "Gnome", "Draenei" };
static const char* kHordeRaces[]    = { "Orc", "Undead", "Tauren", "Troll", "Blood Elf" };
static const char* kClasses[] = {
    "Warrior", "Paladin", "Hunter", "Rogue", "Priest",
    "Death Knight", "Shaman", "Mage", "Warlock", "Druid"
};

static void DrawEditPopup(const std::string& popupId, CharacterInfo& ch, MetaStore* metaStore) {
    if (ImGui::BeginPopup(popupId.c_str())) {
        static std::string editingKey;
        static int factionIdx = 0;
        static int raceIdx = 0;
        static int classIdx = 0;

        std::string thisKey = ch.account + "|" + ch.realm + "|" + ch.name;
        if (editingKey != thisKey) {

            editingKey = thisKey;
            factionIdx = (ch.faction == Faction::Alliance) ? 1 : (ch.faction == Faction::Horde) ? 2 : 0;
            raceIdx = 0;
            classIdx = 0;
            const char** raceList = (factionIdx == 2) ? kHordeRaces : kAllianceRaces;
            int raceCount = (factionIdx == 2) ? 5 : 5;
            for (int i = 0; i < raceCount; ++i) if (ch.race == raceList[i]) raceIdx = i;
            for (int i = 0; i < 10; ++i) if (ch.wowClass == kClasses[i]) classIdx = i;
        }

        ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.25f, 1.0f), "%s", ch.name.c_str());
        ImGui::Separator();

        const char* factionNames[] = { "Unknown", "Alliance", "Horde" };
        ImGui::SetNextItemWidth(160);
        if (ImGui::Combo("Faction", &factionIdx, factionNames, 3)) {
            raceIdx = 0;
        }

        const char** raceList = (factionIdx == 2) ? kHordeRaces : kAllianceRaces;
        ImGui::SetNextItemWidth(160);
        ImGui::Combo("Race", &raceIdx, raceList, 5);

        ImGui::SetNextItemWidth(160);
        ImGui::Combo("Class", &classIdx, kClasses, 10);

        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            CharMeta meta;
            meta.faction  = (factionIdx == 1) ? Faction::Alliance
                           : (factionIdx == 2) ? Faction::Horde
                           : Faction::Unknown;
            meta.race     = raceList[raceIdx];
            meta.wowClass = kClasses[classIdx];

            ch.faction         = meta.faction;
            ch.race            = meta.race;
            ch.wowClass        = meta.wowClass;
            ch.factionDetected = (meta.faction != Faction::Unknown);
            ch.metaIsManual    = true;

            if (metaStore) metaStore->Set(ch.account, ch.realm, ch.name, meta);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void DrawAccountColumn(const char* label, WowInstall& install,
                               int& selAccount, int& selChar, MetaStore* metaStore) {
    ImGui::BeginGroup();
    ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.25f, 1.0f), "%s", label);
    ImGui::BeginChild(label, ImVec2(420, 380), true);

    for (int ai = 0; ai < (int)install.accounts.size(); ++ai) {
        AccountInfo& acc = install.accounts[ai];
        bool accOpen = ImGui::TreeNodeEx(acc.name.c_str(),
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);

        if (accOpen) {
            bool accSelected = (selAccount == ai && selChar == -1);
            if (ImGui::Selectable("  [account-wide only]", accSelected)) {
                selAccount = ai;
                selChar = -1;
            }

            std::string lastRealm;
            for (int ci = 0; ci < (int)acc.characters.size(); ++ci) {
                CharacterInfo& ch = acc.characters[ci];
                if (ch.realm != lastRealm) {
                    ImGui::TextDisabled("  %s", ch.realm.c_str());
                    lastRealm = ch.realm;
                }
                ImVec4 col = Theme::FactionColor((int)ch.faction);
                ImGui::PushStyleColor(ImGuiCol_Text, col);

                std::string tag;
                if (ch.faction != Faction::Unknown && !ch.race.empty() && !ch.wowClass.empty()) {
                    tag = "[" + std::string(FactionName(ch.faction)) + " " + ch.race + " " + ch.wowClass + "]";
                } else {
                    tag = std::string("[Unknown") + (ch.metaIsManual ? "]" : "?]");
                }
                std::string labelStr = "    " + ch.name + "  " + tag;

                bool chSelected = (selAccount == ai && selChar == ci);
                if (ImGui::Selectable(labelStr.c_str(), chSelected,
                                       ImGuiSelectableFlags_AllowDoubleClick)) {
                    selAccount = ai;
                    selChar = ci;
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                std::string editBtnId = "..##edit_" + std::string(label) + std::to_string(ai) + "_" + std::to_string(ci);
                std::string popupId = "editpopup_" + std::string(label) + std::to_string(ai) + "_" + std::to_string(ci);
                if (ImGui::SmallButton(editBtnId.c_str())) {
                    ImGui::OpenPopup(popupId.c_str());
                }
                DrawEditPopup(popupId, ch, metaStore);
            }
            ImGui::TreePop();
        }
    }

    ImGui::EndChild();
    ImGui::EndGroup();
}

void DrawFrame(AppState& state) {
    Theme::Apply();

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::Begin("WoW CFG Changer", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.25f, 1.0f), "WoW 3.3.5a Config Migrator");
    ImGui::Separator();

    ImGui::Text("WoW install folder (contains WTF/, Interface/):");
    ImGui::SetNextItemWidth(600);
    ImGui::InputText("##rootpath", rootPathBuf, sizeof(rootPathBuf));
    ImGui::SameLine();
    if (ImGui::Button("SCAN FOLDERS")) {

        state.install.root = Utf8::Utf8ToPath(rootPathBuf);
        state.log.clear();
        state.scanned = WowScan::ScanInstall(state.install, state.log);
        if (state.scanned && state.metaStore) {

            state.metaStore->ApplyOverrides(state.install);
        }
        state.selectedSrcAccount = state.selectedSrcChar = -1;
        state.selectedDstAccount = state.selectedDstChar = -1;
    }

    ImGui::Spacing();

    if (state.scanned) {
        DrawAccountColumn("SOURCE (copy FROM)", state.install,
                           state.selectedSrcAccount, state.selectedSrcChar, state.metaStore);
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(20, 0));
        ImGui::SameLine();
        DrawAccountColumn("DESTINATION (copy TO)", state.install,
                           state.selectedDstAccount, state.selectedDstChar, state.metaStore);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("What to copy:");
        ImGui::Checkbox("Client settings (config-cache.wtf)", &state.options.copyClientConfig);
        ImGui::SameLine(300);
        ImGui::Checkbox("Macros", &state.options.copyMacros);
        ImGui::Checkbox("Keybinds", &state.options.copyKeybinds);
        ImGui::SameLine(300);
        ImGui::Checkbox("UI layout / Edit Mode", &state.options.copyLayout);
        ImGui::Checkbox("Addon SavedVariables (character)", &state.options.copyAddonSaved);
        ImGui::SameLine(300);
        ImGui::Checkbox("Addon SavedVariables (account-wide, e.g. WeakAuras)", &state.options.copyAccountWideSV);
        ImGui::Checkbox("Overwrite existing files", &state.options.overwriteExisting);

        ImGui::Spacing();
        bool canCopy = state.selectedSrcAccount >= 0 && state.selectedDstAccount >= 0;
        if (!canCopy) ImGui::BeginDisabled();
        if (ImGui::Button("COPY CONFIG NOW", ImVec2(220, 40))) {
            AccountInfo& srcAcc = state.install.accounts[state.selectedSrcAccount];
            AccountInfo& dstAcc = state.install.accounts[state.selectedDstAccount];
            CharacterInfo* srcChar = (state.selectedSrcChar >= 0)
                ? &srcAcc.characters[state.selectedSrcChar] : nullptr;
            CharacterInfo* dstChar = (state.selectedDstChar >= 0)
                ? &dstAcc.characters[state.selectedDstChar] : nullptr;

            AppLogic::CopyConfig(srcChar, srcAcc, dstChar, dstAcc,
                                  state.options, state.log);
        }
        if (!canCopy) ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Log:");
        ImGui::BeginChild("log", ImVec2(0, 160), true);
        for (auto& line : state.log) ImGui::TextUnformatted(line.c_str());
        if (!state.log.empty()) ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
    } else if (!state.log.empty()) {
        for (auto& line : state.log)
            ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "%s", line.c_str());
    }

    ImGui::End();
}

}
