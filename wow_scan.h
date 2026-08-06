#pragma once
#include "app_state.h"

namespace WowScan {

    bool ScanInstall(WowInstall& install, std::vector<std::string>& log);

    void DetectFaction(CharacterInfo& c);

}
