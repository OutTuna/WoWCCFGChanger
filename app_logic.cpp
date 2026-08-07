#include "app_logic.h"
#include "utf8_utils.h"
#include <iostream>

namespace AppLogic {

static void CopyFileIfExists(const fs::path& src, const fs::path& dst,
                              bool overwrite, std::vector<std::string>& log) {
    if (!fs::exists(src)) return;
    try {
        fs::create_directories(dst.parent_path());

        if (fs::exists(dst)) {
            if (!overwrite) {
                log.push_back("    skipped (already exists): " + Utf8::PathToUtf8(src.filename()));
                return;
            }
            fs::remove(dst);
        }

        fs::copy_file(src, dst);
        log.push_back("    copied: " + Utf8::PathToUtf8(src.filename()));
    } catch (const std::exception& e) {
        log.push_back("    [!] failed to copy " + Utf8::PathToUtf8(src.filename()) + ": " + e.what());
    }
}

static void CopyDirIfExists(const fs::path& src, const fs::path& dst,
                             bool overwrite, std::vector<std::string>& log) {
    if (!fs::exists(src) || !fs::is_directory(src)) return;
    fs::create_directories(dst);
    for (auto& entry : fs::recursive_directory_iterator(src)) {
        auto rel = fs::relative(entry.path(), src);
        fs::path target = dst / rel;
        if (entry.is_directory()) {
            fs::create_directories(target);
        } else if (entry.is_regular_file()) {
            CopyFileIfExists(entry.path(), target, overwrite, log);
        }
    }
}

bool CopyConfig(const CharacterInfo* srcChar,
                 const AccountInfo&  srcAccount,
                 CharacterInfo*       dstChar,
                 const AccountInfo&  dstAccount,
                 const CopyOptions&   opts,
                 std::vector<std::string>& log) {

    log.push_back("=== Starting copy ===");

    if (opts.copyClientConfig) {
        CopyFileIfExists(srcAccount.path / "config-cache.wtf",
                          dstAccount.path / "config-cache.wtf",
                          opts.overwriteExisting, log);
    }
    if (opts.copyAccountWideSV) {
        CopyDirIfExists(srcAccount.path / "SavedVariables",
                         dstAccount.path / "SavedVariables",
                         opts.overwriteExisting, log);
    }

    if (srcChar && dstChar) {
        log.push_back("Character: " + srcChar->name + "-" + srcChar->realm +
                       "  ->  " + dstChar->name + "-" + dstChar->realm);

        if (opts.copyClientConfig) {
            CopyFileIfExists(srcChar->path / "config-cache.wtf",
                              dstChar->path / "config-cache.wtf",
                              opts.overwriteExisting, log);
        }
        if (opts.copyMacros) {
            CopyFileIfExists(srcChar->path / "macros-cache.txt",
                              dstChar->path / "macros-cache.txt",
                              opts.overwriteExisting, log);
        }
        if (opts.copyKeybinds) {
            CopyFileIfExists(srcChar->path / "bindings-cache.wtf",
                              dstChar->path / "bindings-cache.wtf",
                              opts.overwriteExisting, log);
        }
        if (opts.copyLayout) {
            CopyFileIfExists(srcChar->path / "layout-local.txt",
                              dstChar->path / "layout-local.txt",
                              opts.overwriteExisting, log);
            CopyFileIfExists(srcChar->path / "edit-mode-cache-account.txt",
                              dstChar->path / "edit-mode-cache-account.txt",
                              opts.overwriteExisting, log);
        }
        if (opts.copyAddonSaved) {
            CopyDirIfExists(srcChar->path / "SavedVariables",
                             dstChar->path / "SavedVariables",
                             opts.overwriteExisting, log);
        }
    } else {
        log.push_back("(account-wide only copy, no character selected)");
    }

    log.push_back("=== Done ===");
    return true;
}

}