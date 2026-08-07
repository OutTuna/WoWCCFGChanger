#include "diff_calc.h"
#include "utf8_utils.h"
#include <system_error>

namespace DiffCalc {

static DiffStatus ClassifyFile(const fs::path& src, const fs::path& dst) {
    std::error_code ec;
    if (!fs::exists(dst, ec)) return DiffStatus::New;

    auto srcSize = fs::file_size(src, ec);
    auto dstSize = fs::file_size(dst, ec);
    if (srcSize != dstSize) return DiffStatus::WillOverwrite;

    auto srcTime = fs::last_write_time(src, ec);
    auto dstTime = fs::last_write_time(dst, ec);
    if (srcTime != dstTime) return DiffStatus::WillOverwrite;

    return DiffStatus::Identical;
}

static void AddFileEntry(std::vector<DiffEntry>& out, const std::string& category,
                          const fs::path& src, const fs::path& dst) {
    if (!fs::exists(src)) return;
    DiffEntry e;
    e.category = category;
    e.label = Utf8::PathToUtf8(src.filename());
    e.status = ClassifyFile(src, dst);
    out.push_back(std::move(e));
}

static void AddDirEntries(std::vector<DiffEntry>& out, const std::string& category,
                           const fs::path& srcDir, const fs::path& dstDir) {
    if (!fs::exists(srcDir) || !fs::is_directory(srcDir)) return;
    for (auto& entry : fs::recursive_directory_iterator(srcDir)) {
        if (!entry.is_regular_file()) continue;
        auto rel = fs::relative(entry.path(), srcDir);
        fs::path dstFile = dstDir / rel;

        DiffEntry e;
        e.category = category;
        e.label = Utf8::PathToUtf8(rel);
        e.status = ClassifyFile(entry.path(), dstFile);
        out.push_back(std::move(e));
    }
}

std::vector<DiffEntry> Compute(const CharacterInfo* srcChar, const AccountInfo& srcAccount,
                                const CharacterInfo* dstChar, const AccountInfo& dstAccount,
                                const CopyOptions& opts) {
    std::vector<DiffEntry> out;

    if (opts.copyClientConfig) {
        AddFileEntry(out, "Client Config",
                      srcAccount.path / "config-cache.wtf",
                      dstAccount.path / "config-cache.wtf");
    }
    if (opts.copyAccountWideSV) {
        AddDirEntries(out, "Account SV",
                       srcAccount.path / "SavedVariables",
                       dstAccount.path / "SavedVariables");
    }

    if (srcChar && dstChar) {
        if (opts.copyClientConfig) {
            AddFileEntry(out, "Client Config",
                          srcChar->path / "config-cache.wtf",
                          dstChar->path / "config-cache.wtf");
        }
        if (opts.copyMacros) {
            AddFileEntry(out, "Macros",
                          srcChar->path / "macros-cache.txt",
                          dstChar->path / "macros-cache.txt");
        }
        if (opts.copyKeybinds) {
            AddFileEntry(out, "Keybinds",
                          srcChar->path / "bindings-cache.wtf",
                          dstChar->path / "bindings-cache.wtf");
        }
        if (opts.copyLayout) {
            AddFileEntry(out, "Layout",
                          srcChar->path / "layout-local.txt",
                          dstChar->path / "layout-local.txt");
            AddFileEntry(out, "Layout",
                          srcChar->path / "edit-mode-cache-account.txt",
                          dstChar->path / "edit-mode-cache-account.txt");
        }
        if (opts.copyAddonSaved) {
            AddDirEntries(out, "Addon SV",
                           srcChar->path / "SavedVariables",
                           dstChar->path / "SavedVariables");
        }
    }

    return out;
}

}
