#include "ui.h"
#include "wow_scan.h"
#include "app_logic.h"
#include "diff_calc.h"
#include "theme.h"
#include "utf8_utils.h"
#include "meta_store.h"
#include "app_config.h"
#include "imgui.h"
#include <cstring>
#include <algorithm>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <shellapi.h>
#endif

namespace UI {
    static const char* kCreatorName = "OutTuna";
    static const char* kRepoUrl     = "https://github.com/OutTuna/WowCFGChanger";

    static void OpenUrlInBrowser(const char* url) {
#ifdef _WIN32
        ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
        std::string cmd = std::string("open \"") + url + "\"";
        system(cmd.c_str());
#else
        std::string cmd = std::string("xdg-open \"") + url + "\"";
        system(cmd.c_str());
#endif
    }

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
                                   int& selAccount, int& selChar, MetaStore* metaStore,
                                   ImVec2 size) {
        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.25f, 1.0f), "%s", label);
        ImGui::BeginChild(label, size, true);

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

                    float editBtnW = 26.0f;
                    float rowAvail = ImGui::GetContentRegionAvail().x;

                    bool chSelected = (selAccount == ai && selChar == ci);
                    if (ImGui::Selectable(labelStr.c_str(), chSelected,
                                           ImGuiSelectableFlags_AllowDoubleClick,
                                           ImVec2(rowAvail - editBtnW - 4.0f, 0))) {
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

    static ImVec4 DiffColor(DiffStatus s) {
        switch (s) {
            case DiffStatus::New:           return ImVec4(0.45f, 0.85f, 0.45f, 1.0f); // green
            case DiffStatus::WillOverwrite: return ImVec4(0.95f, 0.70f, 0.25f, 1.0f); // amber
            default:                        return ImVec4(0.55f, 0.55f, 0.55f, 1.0f); // grey
        }
    }
    static const char* DiffLabel(DiffStatus s) {
        switch (s) {
            case DiffStatus::New:           return "NEW";
            case DiffStatus::WillOverwrite: return "OVERWRITE";
            default:                        return "same";
        }
    }
    static void DrawDiffColumn(AppState& state, ImVec2 size) {
        static std::vector<DiffEntry> cache;
        static int lastSrcAcc = -2, lastSrcChar = -2, lastDstAcc = -2, lastDstChar = -2;
        static CopyOptions lastOpts{};

        bool selectionChanged =
            lastSrcAcc != state.selectedSrcAccount || lastSrcChar != state.selectedSrcChar ||
            lastDstAcc != state.selectedDstAccount || lastDstChar != state.selectedDstChar;

        bool optsChanged =
            lastOpts.copyClientConfig != state.options.copyClientConfig ||
            lastOpts.copyMacros       != state.options.copyMacros ||
            lastOpts.copyKeybinds     != state.options.copyKeybinds ||
            lastOpts.copyLayout       != state.options.copyLayout ||
            lastOpts.copyAddonSaved   != state.options.copyAddonSaved ||
            lastOpts.copyAccountWideSV != state.options.copyAccountWideSV;

        bool haveSelection = state.selectedSrcAccount >= 0 && state.selectedDstAccount >= 0;

        if (haveSelection && (selectionChanged || optsChanged)) {
            AccountInfo& srcAcc = state.install.accounts[state.selectedSrcAccount];
            AccountInfo& dstAcc = state.install.accounts[state.selectedDstAccount];
            CharacterInfo* srcChar = (state.selectedSrcChar >= 0) ? &srcAcc.characters[state.selectedSrcChar] : nullptr;
            CharacterInfo* dstChar = (state.selectedDstChar >= 0) ? &dstAcc.characters[state.selectedDstChar] : nullptr;

            cache = DiffCalc::Compute(srcChar, srcAcc, dstChar, dstAcc, state.options);

            lastSrcAcc = state.selectedSrcAccount; lastSrcChar = state.selectedSrcChar;
            lastDstAcc = state.selectedDstAccount; lastDstChar = state.selectedDstChar;
            lastOpts = state.options;
        } else if (!haveSelection) {
            cache.clear();
            lastSrcAcc = lastDstAcc = -2; lastSrcChar = lastDstChar = -2;
        }

        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.25f, 1.0f), "DIFF PREVIEW");
        ImGui::BeginChild("diffcol", size, true);

        if (!haveSelection) {
            ImGui::TextDisabled("Select a source and destination to preview changes.");
        } else if (cache.empty()) {
            ImGui::TextDisabled("Nothing to copy with current options.");
        } else {
            int newCount = 0, overwriteCount = 0, sameCount = 0;
            for (auto& e : cache) {
                if (e.status == DiffStatus::New) newCount++;
                else if (e.status == DiffStatus::WillOverwrite) overwriteCount++;
                else sameCount++;
            }
            ImGui::TextColored(ImVec4(0.45f, 0.85f, 0.45f, 1.0f), "%d new", newCount);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.95f, 0.70f, 0.25f, 1.0f), "%d overwrite", overwriteCount);
            ImGui::SameLine();
            ImGui::TextDisabled("%d unchanged", sameCount);
            ImGui::Separator();

            std::string lastCategory;
            for (auto& e : cache) {
                if (e.category != lastCategory) {
                    ImGui::TextDisabled("%s", e.category.c_str());
                    lastCategory = e.category;
                }
                ImGui::TextColored(DiffColor(e.status), "  [%s]", DiffLabel(e.status));
                ImGui::SameLine();
                ImGui::TextUnformatted(e.label.c_str());
            }
        }

        ImGui::EndChild();
        ImGui::EndGroup();
    }

    static void DrawAboutPopup() {
        if (ImGui::BeginPopup("AboutPopup")) {
            ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.25f, 1.0f), "WoW CFG Changer");
            ImGui::TextDisabled("v1.0 — C++ / ImGui / GLFW");
            ImGui::Separator();

            ImGui::Text("Created by %s", kCreatorName);
            ImGui::Spacing();

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 320.0f);
            ImGui::TextColored(ImVec4(0.95f, 0.55f, 0.35f, 1.0f),
                "This program is free and open-source. If you paid money for it, "
                "you were scammed — get a refund and grab the real thing from "
                "the link below.");
            ImGui::PopTextWrapPos();

            ImGui::Spacing();
            if (ImGui::Button("Open GitHub repo", ImVec2(220, 0))) {
                OpenUrlInBrowser(kRepoUrl);
            }
            ImGui::SameLine();
            if (ImGui::Button("Close", ImVec2(100, 0))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void DrawFrame(AppState& state) {
        Theme::Apply();

        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::Begin("WoW CFG Changer", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.25f, 1.0f), "WoW 3.3.5a Config Migrator");
        ImGui::SameLine();
        {
            float btnSize = 28.0f;
            float rightEdge = ImGui::GetWindowContentRegionMax().x;
            ImGui::SetCursorPosX(rightEdge - btnSize);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);

            bool clicked;
            if (state.aboutIconLoaded) {
                clicked = ImGui::ImageButton("about_btn",
                    (ImTextureID)(intptr_t)state.aboutIconTexture,
                    ImVec2(btnSize, btnSize));
            } else {
                clicked = ImGui::Button("i##about_btn", ImVec2(btnSize, btnSize));
            }
            if (clicked) {
                ImGui::OpenPopup("AboutPopup");
            }
            DrawAboutPopup();
        }
        ImGui::Separator();

        ImGui::Text("WoW install folder (contains WTF/, Interface/):");
        ImGui::SetNextItemWidth(600);
        ImGui::InputText("##rootpath", state.rootPathBuf, sizeof(state.rootPathBuf));
        ImGui::SameLine();
        if (ImGui::Button("SCAN FOLDERS")) {
            state.install.root = Utf8::Utf8ToPath(state.rootPathBuf);
            state.log.clear();
            state.scanned = WowScan::ScanInstall(state.install, state.log);
            if (state.scanned && state.metaStore) {
                state.metaStore->ApplyOverrides(state.install);
            }
            if (state.scanned && state.appConfig) {
                state.appConfig->SetLastRootPath(state.rootPathBuf);
            }
            state.selectedSrcAccount = state.selectedSrcChar = -1;
            state.selectedDstAccount = state.selectedDstChar = -1;
        }

        ImGui::Spacing();

        if (state.scanned) {
            float gap = 20.0f;
            float totalW = ImGui::GetContentRegionAvail().x;
            float colW = (totalW - gap * 2.0f) / 3.0f;
            ImVec2 colSize(colW, 300);

            DrawAccountColumn("SOURCE (copy FROM)", state.install,
                               state.selectedSrcAccount, state.selectedSrcChar, state.metaStore, colSize);
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(gap, 0));
            ImGui::SameLine();
            DrawAccountColumn("DESTINATION (copy TO)", state.install,
                               state.selectedDstAccount, state.selectedDstChar, state.metaStore, colSize);
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(gap, 0));
            ImGui::SameLine();
            DrawDiffColumn(state, colSize);

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

            ImVec2 logSize(0, std::max(140.0f, ImGui::GetContentRegionAvail().y));
            ImGui::BeginChild("log", logSize, true, ImGuiWindowFlags_HorizontalScrollbar);

            for (auto& line : state.log) ImGui::TextUnformatted(line.c_str());
            static int lastLogSize = -1;
            if ((int)state.log.size() != lastLogSize) {
                ImGui::SetScrollHereY(1.0f);
                lastLogSize = (int)state.log.size();
            }
            ImGui::EndChild();
        } else if (!state.log.empty()) {
            for (auto& line : state.log)
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "%s", line.c_str());
        }

        ImGui::End();
    }
}