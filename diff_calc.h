#pragma once
#include "app_state.h"
#include <vector>
#include <string>

enum class DiffStatus { New, WillOverwrite, Identical };

struct DiffEntry {
    std::string category;
    std::string label;
    DiffStatus  status;
};

namespace DiffCalc {
    std::vector<DiffEntry> Compute(const CharacterInfo* srcChar, const AccountInfo& srcAccount,
                                    const CharacterInfo* dstChar, const AccountInfo& dstAccount,
                                    const CopyOptions& opts);
}
