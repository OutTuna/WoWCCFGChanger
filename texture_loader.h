#pragma once
#include <filesystem>

namespace fs = std::filesystem;

namespace TextureLoader {
    unsigned int LoadPngAsTexture(const fs::path& file, int& outW, int& outH);
}
