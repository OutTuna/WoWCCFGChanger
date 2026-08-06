#pragma once
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace Utf8 {
    std::string PathToUtf8(const fs::path& p);
    fs::path Utf8ToPath(const std::string& utf8);

}
