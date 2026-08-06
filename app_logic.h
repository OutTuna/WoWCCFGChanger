#pragma once
#include "app_state.h"

namespace AppLogic {

    bool CopyConfig(const CharacterInfo* srcChar,
                     const AccountInfo&  srcAccount,
                     CharacterInfo*       dstChar,
                     const AccountInfo&  dstAccount,
                     const CopyOptions&   opts,
                     std::vector<std::string>& log);

}
